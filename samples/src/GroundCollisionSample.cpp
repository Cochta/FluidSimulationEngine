#include "GroundCollisionSample.h"

std::string GroundCollisionSample::GetName() noexcept {
	return "Bouncing Ground";
}

std::string GroundCollisionSample::GetDescription() noexcept {
	return "CONTROLS: Left click to create a sphere, right click to create a "
		"cuboid.\n\nFloor has 1 x bounciness, spheres and cuboids have "
		"0 x bounciness";
}

void GroundCollisionSample::OnCollisionEnter(ColliderRef col1,
	ColliderRef col2) noexcept {
}

void GroundCollisionSample::OnCollisionExit(ColliderRef col1,
	ColliderRef col2) noexcept {
}

void GroundCollisionSample::SampleSetUp() noexcept {
	_world.SetContactListener(this);

	// Create static cuboid
	const auto groundRef = _world.CreateBody();
	_bodyRefs.push_back(groundRef);
	auto& groundBody = _world.GetBody(groundRef);
	groundBody.Type = BodyType::STATIC;
	groundBody.Mass = 1;

	groundBody.Position = { 0,Metrics::MetersToPixels(-1),0 };

	const auto groundColRef = _world.CreateCollider(groundRef);
	_colRefs.push_back(groundColRef);
	auto& groundCol = _world.GetCollider(groundColRef);
	groundCol.Shape = CuboidF({ Metrics::MetersToPixels(-3), Metrics::MetersToPixels(-0.1f), Metrics::MetersToPixels(-3) },
		{ Metrics::MetersToPixels(3), Metrics::MetersToPixels(0.1f), Metrics::MetersToPixels(3) });
	groundCol.BodyPosition = groundBody.Position;
	groundCol.Restitution = 1.f;

	AllGraphicsData.emplace_back();
}
void GroundCollisionSample::SampleUpdate() noexcept {
	if (_mouseLeftReleased) {
		CreateBall(_mousePos);
	}
	else if (_mouseRightReleased) {
		CreateRect(_mousePos);
	}

	for (std::size_t i = 0; i < _colRefs.size(); ++i) {
		const auto& col = _world.GetCollider(_colRefs[i]);

		const auto& shape = _world.GetCollider(_colRefs[i]).Shape;

		switch (shape.index()) {
		case static_cast<int>(ShapeType::Sphere):
			_world.GetBody(col.BodyRef).ApplyForce({ 0, SPEED });
			AllGraphicsData[i].Shape =
				std::get<SphereF>(shape) + col.BodyPosition;
			break;
		case static_cast<int>(ShapeType::Cuboid):
			if (i != 0) {
				_world.GetBody(col.BodyRef).ApplyForce({ 0, SPEED });
			}
			AllGraphicsData[i].Shape =
				std::get<CuboidF>(shape) + col.BodyPosition;
			break;
		default:
			break;
		}
	}
}

void GroundCollisionSample::SampleTearDown() noexcept {}

void GroundCollisionSample::CreateBall(XMVECTOR position) noexcept {
	const auto sphereBodyRef = _world.CreateBody();
	_bodyRefs.push_back(sphereBodyRef);
	auto& sphereBody = _world.GetBody(sphereBodyRef);

	sphereBody.Mass = 1;

	//sphereBody.Position = position;
	sphereBody.Position = XMVectorZero();

	const auto sphereColRef = _world.CreateCollider(sphereBodyRef);
	_colRefs.push_back(sphereColRef);
	auto& sphereCol = _world.GetCollider(sphereColRef);
	sphereCol.Shape =
		Sphere(XMVectorZero(), Random::Range(10.f, 40.f));
	sphereCol.BodyPosition = sphereBody.Position;
	sphereCol.Restitution = 0.f;

	GraphicsData gd;
	gd.Color = { Random::Range(0, 255), Random::Range(0, 255),
				Random::Range(0, 255), 255 };

	AllGraphicsData.emplace_back(gd);
}

void GroundCollisionSample::CreateRect(XMVECTOR position) noexcept {
	const auto rectBodyRef = _world.CreateBody();
	_bodyRefs.push_back(rectBodyRef);
	auto& rectBody = _world.GetBody(rectBodyRef);
	rectBody.Type = BodyType::DYNAMIC;

	//rectBody.Position = position;
	rectBody.Position = XMVectorZero();

	const auto rectColRef = _world.CreateCollider(rectBodyRef);
	_colRefs.push_back(rectColRef);
	auto& rectCol = _world.GetCollider(rectColRef);
	rectCol.Shape = CuboidF({ -Random::Range(1.f, 4.f) * 10.f, -Random::Range(1.f, 4.f) * 10.f,-Random::Range(1.f, 4.f) * 10.f },
		{ Random::Range(1.f, 4.f) * 10.f, Random::Range(1.f, 4.f) * 10.f, Random::Range(1.f, 4.f) * 10.f });
	rectCol.BodyPosition = rectBody.Position;
	rectCol.Restitution = 0.f;

	GraphicsData gd;
	gd.Color = { Random::Range(0, 255), Random::Range(0, 255),
				Random::Range(0, 255), 255 };

	AllGraphicsData.emplace_back(gd);
}
