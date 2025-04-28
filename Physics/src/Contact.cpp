#include "Contact.h"

void Contact::Resolve()
{
	switch (CollidingBodies[0].collider->Shape.index())
	{
	case static_cast<int>(ShapeType::Sphere):
	switch (CollidingBodies[1].collider->Shape.index())
	{
	case static_cast<int>(ShapeType::Sphere):
	{
		const SphereF& sphere0 = std::get<SphereF>(CollidingBodies[0].collider->Shape);
		const SphereF& sphere1 = std::get<SphereF>(CollidingBodies[1].collider->Shape);

		const auto delta = XMVectorSubtract(XMVectorSubtract(XMVectorAdd(CollidingBodies[0].body->Position, sphere0.Center()), CollidingBodies[1].body->Position), sphere1.Center());

		float length = XMVectorGetX(XMVector3Length(delta));

		if (length > 0.f)
		{
			Normal = XMVector3Normalize(delta);
		}
		else
		{
			Normal = g_XMIdentityR1; // vector up
		}
		Penetration = std::get<SphereF>(CollidingBodies[0].collider->Shape).Radius() +
			std::get<SphereF>(CollidingBodies[1].collider->Shape).Radius() - length;
	}
	break;
	case static_cast<int>(ShapeType::Cuboid):
	{
		const SphereF& sphere = std::get<SphereF>(CollidingBodies[0].collider->Shape);
		const CuboidF& cuboid = std::get<CuboidF>(CollidingBodies[1].collider->Shape);



		auto closestX = Clamp(
			XMVectorGetX(CollidingBodies[0].body->Position) + XMVectorGetX(sphere.Center()),
			XMVectorGetX(cuboid.MinBound()) + XMVectorGetX(CollidingBodies[1].body->Position),
			XMVectorGetX(cuboid.MaxBound()) + XMVectorGetX(CollidingBodies[1].body->Position)
		);

		auto closestY = Clamp(
			XMVectorGetY(CollidingBodies[0].body->Position) + XMVectorGetY(sphere.Center()),
			XMVectorGetY(cuboid.MinBound()) + XMVectorGetY(CollidingBodies[1].body->Position),
			XMVectorGetY(cuboid.MaxBound()) + XMVectorGetY(CollidingBodies[1].body->Position)
		);
		auto closestZ = Clamp(
			XMVectorGetZ(CollidingBodies[0].body->Position) + XMVectorGetZ(sphere.Center()),
			XMVectorGetZ(cuboid.MinBound()) + XMVectorGetZ(CollidingBodies[1].body->Position),
			XMVectorGetZ(cuboid.MaxBound()) + XMVectorGetZ(CollidingBodies[1].body->Position)
		);

		const XMVECTOR closest = XMVectorSet(closestX, closestY, closestZ, 0);

		//const XMVECTOR delta = CollidingBodies[0].body->Position + sphere.Center() - closest;

		XMVECTOR delta = XMVectorSubtract(
			XMVectorAdd(CollidingBodies[0].body->Position, sphere.Center()),
			closest
		);

		const float distance = XMVectorGetX(XMVector3Length(delta));

		Penetration = sphere.Radius() - distance;

		if (distance > 0.f)
		{
			Normal = XMVector3Normalize(delta);
		}
		else
		{
			//Normal = g_XMIdentityR1;
			Normal = -XMVector3Normalize(CollidingBodies[0].body->Velocity);
		}

	}
	break;
	}
	break;
	case static_cast<int>(ShapeType::Cuboid):
	switch (CollidingBodies[1].collider->Shape.index())
	{
	case static_cast<int>(ShapeType::Sphere):
	{
		std::swap(CollidingBodies[0], CollidingBodies[1]);
		Resolve();
	}
	break;
	case static_cast<int>(ShapeType::Cuboid):
	{
		const CuboidF& cuboid0 = std::get<CuboidF>(CollidingBodies[0].collider->Shape);
		const CuboidF& cuboid1 = std::get<CuboidF>(CollidingBodies[1].collider->Shape);

		// Calculate delta vector between the centers of the two cuboids
		const auto delta = XMVectorSubtract(
			XMVectorSubtract(
			XMVectorAdd(CollidingBodies[0].body->Position, cuboid0.Center()),
			CollidingBodies[1].body->Position
		),
			cuboid1.Center()
		);

		// Calculate the penetration depth along each axis (x, y, z)
		const XMVECTOR penetration = XMVectorSubtract(
			XMVectorAdd(cuboid0.HalfSize(), cuboid1.HalfSize()),
			XMVectorAbs(delta)
		);

		// Find the smallest penetration axis (x, y, or z)
		if (XMVectorGetX(penetration) < XMVectorGetY(penetration) && XMVectorGetX(penetration) < XMVectorGetZ(penetration)) {
			Penetration = XMVectorGetX(penetration);
			Normal = (XMVectorGetX(delta) > 0) ? XMVectorSet(1.0f, 0.0f, 0.0f, 0) : XMVectorSet(-1.0f, 0.0f, 0.0f, 0);
		}
		else if (XMVectorGetY(penetration) < XMVectorGetZ(penetration)) {
			Penetration = XMVectorGetY(penetration);
			Normal = (XMVectorGetY(delta) > 0) ? XMVectorSet(0.0f, 1.0f, 0.0f, 0) : XMVectorSet(0.0f, -1.0f, 0.0f, 0);
		}
		else {
			Penetration = XMVectorGetZ(penetration);
			Normal = (XMVectorGetZ(delta) > 0) ? XMVectorSet(0.0f, 0.0f, 1.0f, 0) : XMVectorSet(0.0f, 0.0f, -1.0f, 0);
		}
	}
	break;
	}
	}

	const auto mass1 = CollidingBodies[0].body->Mass, mass2 = CollidingBodies[1].body->Mass;
	const auto rest1 = CollidingBodies[0].collider->Restitution, rest2 = CollidingBodies[1].collider->Restitution;

	Restitution = (mass1 * rest1 + mass2 * rest2) / (mass1 + mass2);

	//printf("rest1 = %f, rest2 = %f, Restitution: %f\n", rest1, rest2, Restitution);

	ResolveVelocityAndInterpenetration();
	ResolveInterpenetration();
}

float Contact::CalculateSeparateVelocity() const noexcept
{
	const auto relativeVelocity = XMVectorSubtract(CollidingBodies[0].body->Velocity, CollidingBodies[1].body->Velocity);
	return XMVectorGetX(XMVector3Dot(relativeVelocity, Normal));
	//return result; // relativeVelocity.Dot(Normal);
}

void Contact::ResolveVelocityAndInterpenetration() const noexcept
{
	const float separatingVelocity = CalculateSeparateVelocity();

	if (separatingVelocity > 0) {
		return;
	}

	const float newSeparatingVelocity = -separatingVelocity * Restitution;

	const float deltaVelocity = newSeparatingVelocity - separatingVelocity;

	const float inverseMass1 = 1.f / CollidingBodies[0].body->Mass;
	const float inverseMass2 = 1.f / CollidingBodies[1].body->Mass;

	const float totalInverseMass = inverseMass1 + inverseMass2;

	if (totalInverseMass <= 0) {
		return;
	}

	const float impulse = deltaVelocity / totalInverseMass;
	const auto impulsePerIMass = XMVectorScale(Normal, impulse);

	if (CollidingBodies[0].body->Type == BodyType::DYNAMIC || CollidingBodies[0].body->Type == BodyType::FLUID)
	{
		CollidingBodies[0].body->Velocity = XMVectorAdd(CollidingBodies[0].body->Velocity, XMVectorScale(impulsePerIMass, inverseMass1));
	}
	if (CollidingBodies[1].body->Type == BodyType::DYNAMIC || CollidingBodies[1].body->Type == BodyType::FLUID)
	{
		CollidingBodies[1].body->Velocity = XMVectorSubtract(CollidingBodies[1].body->Velocity, XMVectorScale(impulsePerIMass, inverseMass2));
	}

	if (CollidingBodies[0].body->Type == BodyType::STATIC)
	{
		CollidingBodies[1].body->Velocity = XMVectorSubtract(CollidingBodies[1].body->Velocity, XMVectorScale(impulsePerIMass, inverseMass1));
	}
	if (CollidingBodies[1].body->Type == BodyType::STATIC)
	{
		CollidingBodies[0].body->Velocity = XMVectorAdd(CollidingBodies[0].body->Velocity, XMVectorScale(impulsePerIMass, inverseMass2));
	}
}

void Contact::ResolveInterpenetration() const noexcept
{
	if (Penetration <= 0) return;

	const float inverseMass1 = 1.f / CollidingBodies[0].body->Mass;
	const float inverseMass2 = 1.f / CollidingBodies[1].body->Mass;
	const float totalInverseMass = inverseMass1 + inverseMass2;

	if (totalInverseMass <= 0) {
		return;
	}

	const auto movePerIMass = XMVectorScale(Normal, (Penetration / totalInverseMass));

	if (CollidingBodies[0].body->Type == BodyType::DYNAMIC || CollidingBodies[0].body->Type == BodyType::FLUID)
	{
		CollidingBodies[0].body->Position = XMVectorAdd(CollidingBodies[0].body->Position, XMVectorScale(movePerIMass, inverseMass1));
	}

	if (CollidingBodies[1].body->Type == BodyType::DYNAMIC || CollidingBodies[1].body->Type == BodyType::FLUID)
	{
		CollidingBodies[1].body->Position = XMVectorSubtract(CollidingBodies[1].body->Position, XMVectorScale(movePerIMass, inverseMass2));
	}
}

