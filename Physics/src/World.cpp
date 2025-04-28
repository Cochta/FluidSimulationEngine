#include "World.h"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#include <TracyC.h>
#endif 

void World::SetUp(int initSize) noexcept
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	_bodies.resize(initSize);
	BodyGenIndices.resize(initSize, 0);

	_particlesData.reserve(initSize);

	_colliders.resize(initSize);
	ColliderGenIndices.resize(initSize, 0);

}

void World::TearDown() noexcept
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	_bodies.clear();
	BodyGenIndices.clear();

	_colliders.clear();
	ColliderGenIndices.clear();

	_colRefPairs.clear();

	_particlesData.clear();
	grid.clear();
}

void World::Update(const float deltaTime) noexcept
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	UpdateBodies(deltaTime);
	updateGrid();

	computeNeighborsDensity();
	computeNeighborsPressure();
	computeNeighborsViscosity();

	//for (auto& particle : _particlesData)
	//{
	//	auto vec = ProcessDensity(particle.first);
	//	particle.second.Density = XMVectorGetX(vec);
	//	particle.second.NearDensity = XMVectorGetY(vec);
	//}

	//for (auto& particle : _particlesData)
	//{
	//	//float safeDensity = (particle.second.Density > 1e-6) ? particle.second.Density : 1e-6;
	//	//GetBody(particle.first).ApplyForce(ProcessPressureForce(particle.first));
	//	//GetBody(particle.first).ApplyForce(ProcessPressureForce(particle.first) / particle.second.Density);
	//	XMVECTOR pressureForce = ProcessPressureForce(particle.first);
	//	GetBody(particle.first).ApplyForce(pressureForce / particle.second.Density);
	//}

	//for (auto& particle : _particlesData)
	//{
	//	GetBody(particle.first).ApplyForce(ProcessViscosityForce(particle.first));
	//}

	//UpdateGlobalCollisions(); // Update global collisions the old way, used for testing purposes

	SetUpQuadTree();
	//UpdateOctTreeFluidDensities(OctTree.Nodes[0]);
	//UpdateOctTreeFluidPressureForces(OctTree.Nodes[0]);
	//UpdateOctTreeFluidViscosity(OctTree.Nodes[0]);

	UpdateOctTreeCollisions(OctTree.Nodes[0]);
	//_fluidBodiesPairs.clear();

}

[[nodiscard]] BodyRef World::CreateBody(BodyType type) noexcept
{
	const auto it = std::find_if(_bodies.begin(), _bodies.end(), [](const Body& body) {
		return !body.IsEnabled(); // Get first disabled body
	});

	if (it != _bodies.end())
	{
		const std::size_t index = std::distance(_bodies.begin(), it);
		const auto bodyRef = BodyRef{ index, BodyGenIndices[index] };
		auto& body = GetBody(bodyRef);
		body.Enable();
		body.Type = type;
		if (type == BodyType::FLUID)
		{
			_particlesData.insert({ bodyRef, ParticleData{} });
		}

		return bodyRef;
	}

	const std::size_t previousSize = _bodies.size();

	_bodies.resize(previousSize * 2, Body());
	BodyGenIndices.resize(previousSize * 2, 0);

	const BodyRef bodyRef = { previousSize, BodyGenIndices[previousSize] };
	auto& body = GetBody(bodyRef);
	body.Enable();
	body.Type = type;
	if (type == BodyType::FLUID)
	{
		_particlesData.insert({ bodyRef, ParticleData{} });
	}
	return bodyRef;

}

void World::DestroyBody(const BodyRef bodyRef)
{
	if (BodyGenIndices[bodyRef.Index] != bodyRef.GenIndex)
	{
		throw std::runtime_error("No body found !");
	}

	_bodies[bodyRef.Index].Disable();
}

[[nodiscard]] Body& World::GetBody(const BodyRef bodyRef)
{
	if (BodyGenIndices[bodyRef.Index] != bodyRef.GenIndex)
	{
		throw std::runtime_error("No body found !");
	}

	return _bodies[bodyRef.Index];
}

ColliderRef World::CreateCollider(const BodyRef bodyRef) noexcept
{
	const auto it = std::find_if(_colliders.begin(), _colliders.end(), [](const Collider& collider) {
		return !collider.IsAttached; // Get first disabled collider
	});

	if (it != _colliders.end())
	{
		const std::size_t index = std::distance(_colliders.begin(), it);
		const auto colRef = ColliderRef{ index, ColliderGenIndices[index] };
		auto& col = GetCollider(colRef);
		col.IsAttached = true;
		col.BodyRef = bodyRef;

		return colRef;
	}

	const std::size_t previousSize = _colliders.size();

	_colliders.resize(previousSize * 2, Collider());
	ColliderGenIndices.resize(previousSize * 2, 0);

	const ColliderRef colRef = { previousSize, ColliderGenIndices[previousSize] };
	auto& col = GetCollider(colRef);
	col.IsAttached = true;
	col.BodyRef = bodyRef;
	return colRef;
}

Collider& World::GetCollider(const ColliderRef colRef)
{
	if (ColliderGenIndices[colRef.Index] != colRef.GenIndex)
	{
		throw std::runtime_error("No collider found !");
	}

	return _colliders[colRef.Index];
}

void World::DestroyCollider(const ColliderRef colRef)
{
	if (ColliderGenIndices[colRef.Index] != colRef.GenIndex)
	{
		throw std::runtime_error("No collider found !");
	}
	_colliders[colRef.Index].IsAttached = false;
}

void World::UpdateBodies(const float deltaTime) noexcept
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	for (auto& body : _bodies)
	{
		if (!body.IsEnabled())
		{
			continue;
		}
		if (body.Type == BodyType::STATIC)
		{
			continue;
		}
		body.ApplyForce({ 0, -Gravity,0 });
		auto acceleration = XMVectorScale(body.GetForce(), 1 / body.Mass);
		//printf("Acceleration: %f %f %f\n", XMVectorGetX(acceleration), XMVectorGetY(acceleration), XMVectorGetZ(acceleration));
		body.Velocity = XMVectorAdd(body.Velocity, XMVectorScale(acceleration, deltaTime));
		body.Position = XMVectorAdd(body.Position, XMVectorScale(body.Velocity, deltaTime));
		body.PredictedPosition = XMVectorAdd(body.Position, XMVectorScale(body.Velocity, deltaTime));
		body.ResetForce();
	}
}

void World::SetUpQuadTree() noexcept {
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

#ifdef TRACY_ENABLE
	ZoneNamedN(SetRoodNodeBoundary, "SetRootNodeBounds", true);
	ZoneValue(_colliders.size());
#endif

	XMVECTOR maxBounds = XMVectorSet(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), 0);
	XMVECTOR minBounds = XMVectorSet(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 0);

	for (auto& collider : _colliders) {
		if (!collider.IsAttached) {
			continue;
		}

		collider.BodyPosition = GetBody(collider.BodyRef).Position;

		const auto bounds = collider.GetBounds();

		minBounds = XMVectorSetX(minBounds, std::min(XMVectorGetX(minBounds), XMVectorGetX(bounds.MinBound())));
		minBounds = XMVectorSetY(minBounds, std::min(XMVectorGetY(minBounds), XMVectorGetY(bounds.MinBound())));
		minBounds = XMVectorSetZ(minBounds, std::min(XMVectorGetZ(minBounds), XMVectorGetZ(bounds.MinBound())));
		maxBounds = XMVectorSetX(maxBounds, std::max(XMVectorGetX(maxBounds), XMVectorGetX(bounds.MaxBound())));
		maxBounds = XMVectorSetY(maxBounds, std::max(XMVectorGetY(maxBounds), XMVectorGetY(bounds.MaxBound())));
		maxBounds = XMVectorSetZ(maxBounds, std::max(XMVectorGetZ(maxBounds), XMVectorGetZ(bounds.MaxBound())));
	}

	OctTree.SetUpRoot(CuboidF(minBounds, maxBounds));

#ifdef TRACY_ENABLE
	ZoneNamedN(Insert, "Insert in OctTree", true);
#endif
	for (std::size_t i = 0; i < _colliders.size(); ++i) {
		if (!_colliders[i].IsAttached) {
			continue;
		}

		OctTree.Insert(OctTree.Nodes[0], { _colliders[i].GetBounds(), { i, ColliderGenIndices[i] } });

	}
}

void World::UpdateOctTreeCollisions(const BVHNode& node) noexcept
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	if (node.Children[0] == nullptr)
	{
		if (node.ColliderRefAabbs.empty())
		{
			return;
		}
		for (std::size_t i = 0; i < node.ColliderRefAabbs.size() - 1; ++i)
		{
			auto& col1 = GetCollider(node.ColliderRefAabbs[i].ColRef);
			auto& body1 = GetBody(col1.BodyRef);

			for (std::size_t j = i + 1; j < node.ColliderRefAabbs.size(); ++j)
			{
				auto& col2 = GetCollider(node.ColliderRefAabbs[j].ColRef);
				auto& body2 = GetBody(col2.BodyRef);

				if (body1.Type == BodyType::FLUID && body2.Type == BodyType::FLUID)
				{
					continue;
				}

				if (!col2.IsTrigger && !col1.IsTrigger) // Physical collision
				{
					if (Overlap(col1, col2))
					{
						Contact contact;
						contact.CollidingBodies[0] = { &GetBody(col1.BodyRef), &col1 };
						contact.CollidingBodies[1] = { &GetBody(col2.BodyRef), &col2 };
						contact.Resolve();
						if (_contactListener != nullptr)
						{
							_contactListener->OnCollisionEnter(node.ColliderRefAabbs[i].ColRef, node.ColliderRefAabbs[j].ColRef);
						}
					}
					else
					{
						if (_contactListener != nullptr)
						{
							_contactListener->OnCollisionExit(node.ColliderRefAabbs[i].ColRef, node.ColliderRefAabbs[j].ColRef);
						}
					}
					continue;
				}

				if (_contactListener == nullptr)
				{
					continue;
				}


				// Trigger collision
				const ColliderRefPair& colPair = { node.ColliderRefAabbs[i].ColRef, node.ColliderRefAabbs[j].ColRef };

				if (_colRefPairs.find(colPair) != _colRefPairs.end())
				{
					if (!Overlap(col1, col2))
					{
						_contactListener->OnTriggerExit(colPair.ColRefA, colPair.ColRefB);
						_colRefPairs.erase(colPair);
					}
					continue;
				}

				if (Overlap(col1, col2))
				{
					_contactListener->OnTriggerEnter(colPair.ColRefA, colPair.ColRefB);
					_colRefPairs.emplace(colPair);
				}
			}
		}
	}
	else
	{
		for (const auto& child : node.Children)
		{
			UpdateOctTreeCollisions(*child);
		}
	}
}

[[nodiscard]] bool World::Overlap(const Collider& colA, const Collider& colB) noexcept
{
	const auto ShapeA = static_cast<ShapeType>(colA.Shape.index());
	const auto ShapeB = static_cast<ShapeType>(colB.Shape.index());

#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	switch (ShapeA)
	{
	case ShapeType::Sphere:
	{
		SphereF sphere = std::get<SphereF>(colA.Shape) + GetBody(colA.BodyRef).Position;
		switch (ShapeB)
		{
		case ShapeType::Sphere:
		return Intersect(sphere, std::get<SphereF>(colB.Shape) + GetBody(colB.BodyRef).Position);
		case ShapeType::Cuboid:
		return Intersect(sphere, std::get<CuboidF>(colB.Shape) + GetBody(colB.BodyRef).Position);
		}
		break;
	}
	case ShapeType::Cuboid:
	{
		CuboidF rect = std::get<CuboidF>(colA.Shape) + GetBody(colA.BodyRef).Position;
		switch (ShapeB)
		{
		case ShapeType::Sphere:
		return Intersect(rect, std::get<SphereF>(colB.Shape) + GetBody(colB.BodyRef).Position);
		case ShapeType::Cuboid:
		return Intersect(rect, std::get<CuboidF>(colB.Shape) + GetBody(colB.BodyRef).Position);
		}
		break;
	}
	}
	return false;
}

void World::UpdateGlobalCollisions() noexcept
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	for (std::size_t i = 0; i < _colliders.size() - 1; ++i)
	{
		ColliderRef colRef1{ i, ColliderGenIndices[i] };
		auto& col1 = GetCollider(colRef1);
		auto& body1 = GetBody(col1.BodyRef);

		col1.BodyPosition = body1.Position;

		if (!col1.IsAttached) continue;

		// Handle solid-body collisions
		for (std::size_t j = i + 1; j < _colliders.size(); ++j)
		{
			ColliderRef colRef2{ j, ColliderGenIndices[j] };
			auto& col2 = GetCollider(colRef2);
			auto& body2 = GetBody(col2.BodyRef);

			if (!col2.IsAttached) continue;

			if (body1.Type == BodyType::FLUID && body2.Type == BodyType::FLUID)
			{
				continue;
			}

			if (!col2.IsTrigger && !col1.IsTrigger) // Physical collision
			{
				if (!Overlap(col1, col2))
				{
					if (_contactListener) _contactListener->OnCollisionExit(colRef1, colRef2);
				}
				else
				{
					Contact contact;
					contact.CollidingBodies[0] = { &body1, &col1 };
					contact.CollidingBodies[1] = { &body2, &col2 };
					contact.Resolve();
					if (_contactListener) _contactListener->OnCollisionEnter(colRef1, colRef2);
				}
				continue;
			}

			if (_contactListener == nullptr)
			{
				return;
			}

			if (_colRefPairs.find({ colRef1, colRef2 }) != _colRefPairs.end())
			{
				if (!Overlap(col1, col2))
				{
					_contactListener->OnTriggerExit(colRef1, colRef2);
					_colRefPairs.erase({ colRef1, colRef2 });
				}
				continue;
			}

			if (Overlap(col1, col2))
			{
				_contactListener->OnTriggerEnter(colRef1, colRef2);
				_colRefPairs.insert({ colRef1, colRef2 });
			}
		}
	}
}

float World::SmoothingKernel(float radius, float distance)
{
	if (distance >= radius)
	{
		return 0;
	}

	float volume = (PI * Pow(radius, 4)) / 6;
	return (radius - distance) * (radius - distance) / volume;
}

float World::SmoothingKernelDerivative(float radius, float distance)
{
	if (distance >= radius)
	{
		return 0;
	}

	float scale = 12 / (Pow(radius, 4) * PI);
	return (distance - radius) * scale;
}

XMVECTOR World::ProcessDensity(BodyRef bodyref)
{
	auto pos = GetBody(bodyref).Position;
	float density = 0;
	float nearDensity = 0;

	float pow2Radius = SPH::SmoothingRadius * SPH::SmoothingRadius;

	//todo: just in radius, use octtree and change it s size with the max radius
	for (auto& particle : _particlesData)
	{

		//auto otherPos = GetBody(particle.first).Position;
		//auto offsetToOther = XMVectorSubtract(otherPos, pos);
		//float distanceToOther = XMVectorGetX(XMVector3Dot(offsetToOther, offsetToOther));

		//// Skip if not within radius
		//if (distanceToOther > pow2Radius) continue;

		//float distance = sqrt(distanceToOther);

		//density += SPH::DensityKernel(SPH::SmoothingRadius, distance);
		//nearDensity += SPH::NearDensityKernel(SPH::SmoothingRadius, distance);

		//old code
		auto& body = GetBody(particle.first);
		float distance = XMVectorGetX(XMVector3Length(XMVectorSubtract(pos, body.Position)));
		float influence = SmoothingKernel(SPH::SmoothingRadius, distance);
		density += influence;

	}
	return XMVectorSet(density, nearDensity, 0, 0);
};

float World::ConvertDensityToPressure(float density)
{
	float densityError = density - SPH::TargetDensity;
	float pressure = densityError * SPH::PressureMultiplier;

	return pressure;
}
float World::ConvertNearDensityToPressure(float nearDensity)
{
	return nearDensity * SPH::nearPressureMultiplier;
}

XMVECTOR World::ProcessPressureForce(BodyRef bodyref)
{
	auto& thisParticle = GetBody(bodyref);//old code

	//auto thisParticle = _particlesData.at(bodyref);
	//float density = thisParticle.Density;
	//float densityNear = thisParticle.NearDensity;
	//float pressure = ConvertDensityToPressure(density);
	//float nearPressure = ConvertNearDensityToPressure(densityNear);
	XMVECTOR pressureForce = XMVectorZero();

	//XMVECTOR pos = GetBody(bodyref).Position;
	//float sqrRadius = SPH::SmoothingRadius * SPH::SmoothingRadius;

	for (auto& particle : _particlesData)
	{
		if (bodyref == particle.first) continue;

		//auto otherParticle = GetBody(particle.first);
		//auto otherPos = otherParticle.Position;
		//auto offsetToOther = otherPos - pos;
		//float sqrDstToOther = XMVectorGetX(XMVector3Dot(offsetToOther, offsetToOther));

		//if (sqrDstToOther > sqrRadius) continue;

		// Calculate pressure force

		//float distance = XMVectorGetX(XMVector3Length(offsetToOther));
		//XMVECTOR direction = (distance > 1e-6) ? XMVectorDivide(offsetToOther, XMVectorReplicate(distance)) : XMVectorZero();

		//float otherDensity = particle.second.Density;
		//float otherDensityNear = particle.second.NearDensity;
		//float otherPressure = ConvertDensityToPressure(otherDensity);
		//float otherNearPressure = ConvertNearDensityToPressure(otherDensityNear);

		//float sharedPressure = CalculateSharedPressure(pressure, otherPressure);
		//float sharedNearPressure = CalculateSharedNearPressure(nearPressure, otherNearPressure);


		//float safeOtherDensity = (otherDensity > 1e-6) ? otherDensity : 1e-6;
		//float safeOtherDensityNear = (otherDensityNear > 1e-6) ? otherDensityNear : 1e-6;

		//pressureForce += direction * SPH::DensityDerivative(SPH::SmoothingRadius, distance) * sharedPressure / safeOtherDensity;
		//pressureForce += direction * SPH::NearDensityDerivative(SPH::SmoothingRadius, distance) * sharedNearPressure / safeOtherDensityNear;


		//old code
		auto& otherParticle = GetBody(particle.first);
		XMVECTOR offset = XMVectorSubtract(thisParticle.Position, otherParticle.Position);
		float distance = XMVectorGetX(XMVector3Length(offset));
		XMVECTOR direction = distance == 0 ? g_XMIdentityR1 : offset / distance;
		float slope = SmoothingKernelDerivative(SPH::SmoothingRadius, distance);
		float density = particle.second.Density;
		float sharedPressure = CalculateSharedPressure(density, _particlesData.at(bodyref).Density);
		pressureForce += sharedPressure * direction * slope * otherParticle.Mass / density;

	}
	return pressureForce;
}

float World::CalculateSharedPressure(float pressure1, float pressure2)
{
	return (ConvertDensityToPressure(pressure1) + ConvertDensityToPressure(pressure2)) * 0.5f;
}
float World::CalculateSharedNearPressure(float nearDensity1, float nearDensity2)
{
	return (ConvertNearDensityToPressure(nearDensity1) + ConvertNearDensityToPressure(nearDensity2)) * 0.5f;
}

//XMVECTOR World::ProcessViscosityForce(BodyRef bodyref)
//{
//	XMVECTOR viscosityForce = XMVectorZero();
//	XMVECTOR position = GetBody(bodyref).Position;
//	for (auto& particle : _particlesData)
//	{
//		float distance = XMVectorGetX(XMVector3Length(XMVectorSubtract(position, GetBody(particle.first).Position)));
//		float influence = SmoothingKernel(SPH::SmoothingRadius, distance);
//		viscosityForce += (GetBody(particle.first).Velocity - GetBody(bodyref).Velocity) * influence;
//	}
//	//printf("ViscosityForce: %f %f %f\n", XMVectorGetX(viscosityForce), XMVectorGetY(viscosityForce), XMVectorGetZ(viscosityForce));
//	return viscosityForce * SPH::ViscosityStrength;
//}

XMVECTOR World::ProcessViscosityForce(BodyRef bodyref)
{
	XMVECTOR viscosityForce = XMVectorZero();
	XMVECTOR position = GetBody(bodyref).Position;
	XMVECTOR velocity = GetBody(bodyref).Velocity;

	for (auto& particle : _particlesData)
	{
		if (particle.first == bodyref) continue; // optional, skip self

		const Body& neighbor = GetBody(particle.first);
		XMVECTOR r = position - neighbor.Position;
		float distance = XMVectorGetX(XMVector3Length(r));

		if (distance > SPH::SmoothingRadius) continue;

		float influence = SmoothingKernel(SPH::SmoothingRadius, distance);
		viscosityForce += (neighbor.Velocity - velocity) * influence * GetBody(bodyref).Mass; // missing mass factor
	}
	//printf("ViscosityForce: %f %f %f\n", XMVectorGetX(viscosityForce), XMVectorGetY(viscosityForce), XMVectorGetZ(viscosityForce));
	return viscosityForce * SPH::ViscosityStrength;
}
