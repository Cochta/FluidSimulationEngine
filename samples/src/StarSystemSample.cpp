#include "StarSystemSample.h"

std::string StarSystemSample::GetName() noexcept
{
	return "Star System";
}

std::string StarSystemSample::GetDescription() noexcept
{
	return "Randomly generated physical objects rotating around a \"sun\" using forces.";
}

void StarSystemSample::SampleSetUp() noexcept
{
	{
		_sunRef = _world.CreateBody();
		auto& sun = _world.GetBody(_sunRef);
		//sun.Position = { static_cast<float>(Metrics::Width) / 2, static_cast<float>(Metrics::Height) / 2 };
		sun.Position = { 0, 0, 0 };
		sun.Mass = 5000;

		_bodyRefs.push_back(_sunRef);
		GraphicsData sgd;
		_spheres.emplace_back(XMVectorZero(), Metrics::MetersToPixels(0.05f));
		sgd.Color = { 255, 255, 0, 255 };
		AllGraphicsData.push_back(sgd);

		for (std::size_t i = 0; i < PLANET_NBR; ++i)
		{
			// Engine
			auto bodyRef = _world.CreateBody();
			auto& body = _world.GetBody(bodyRef);
			//body.Position = { Random::Range(100.f, Metrics::Width - 100.f),Random::Range(100.f, Metrics::Height - 100.f) };
			body.Position = { Random::Range(-25.f, 25.f),Random::Range(-25.f, 25.f), Random::Range(-25.f, 25.f) };

			auto r = XMVectorSubtract(_world.GetBody(_sunRef).Position, body.Position);
			auto v = sqrt(G * (_world.GetBody(_sunRef).Mass / XMVectorGetX(XMVector3Length(r))));

			auto velocityDir = XMVector3Cross(r, XMVectorSet(0.f, 0.f, 1.f, 0.f)); // Cross product with an arbitrary vector (z-axis)
			if (XMVector3Equal(XMVector3Length(velocityDir), XMVectorZero())) {
				// Handle edge case: if `r` is parallel to the z-axis, choose a different axis for the cross product
				velocityDir = XMVector3Cross(r, XMVectorSet(0.f, 1.f, 0.f, 0.f)); // Cross with y-axis
			}
			body.Velocity = XMVectorScale(XMVector3Normalize(velocityDir), v);
			//body.Velocity = XMVectorScale(XMVector3Normalize(XMVectorSet(-XMVectorGetY(r), XMVectorGetX(r), 0, 0)), v);

			body.Mass = 10.f;

			// Graphics
			_bodyRefs.push_back(bodyRef);
			GraphicsData gd;
			_spheres.emplace_back(XMVectorZero(),
				Random::Range(
					Metrics::MetersToPixels(0.0025f),
					Metrics::MetersToPixels(0.005f)));
			gd.Color = {
					Random::Range(0, 255),
					Random::Range(0, 255),
					Random::Range(0, 255),
					255 };

			AllGraphicsData.push_back(gd);
		}
	}
}

void StarSystemSample::CalculateGravitationalForce(const Body& sun, Body& body) noexcept
{

	auto m1m2 = sun.Mass * body.Mass;
	auto r = XMVectorGetX(XMVector3Length(XMVectorSubtract(sun.Position, body.Position)));
	auto r2 = r * r;
	auto F = G * (m1m2 / r2);

	XMVECTOR forceDirection = XMVector3Normalize(XMVectorSubtract(sun.Position, body.Position));
	XMVECTOR force = XMVectorScale(forceDirection, F);
	body.ApplyForce(force);

}

void StarSystemSample::SampleUpdate() noexcept
{
	auto& sun = _world.GetBody(_sunRef);

	for (std::size_t i = 0; i < _bodyRefs.size(); ++i)
	{
		auto& body = _world.GetBody(_bodyRefs[i]);

		AllGraphicsData[i].Shape = _spheres[i] + body.Position;

		if (_bodyRefs[i] == _sunRef) continue; // Skip the Sun

		CalculateGravitationalForce(sun, body);

	}
}

void StarSystemSample::SampleTearDown() noexcept
{
	_spheres.clear();
}

