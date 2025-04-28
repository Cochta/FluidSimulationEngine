#include "TriggerSample.h"

std::string TriggerSample::GetName() noexcept
{
	return "Trigger";
}

std::string TriggerSample::GetDescription() noexcept
{
	return "Randomly generated objects, they become green if they detect a trigger otherwise they stay blue, the trigger detection uses a QuadTree. ";
}

void TriggerSample::OnTriggerEnter(ColliderRef col1, ColliderRef col2) noexcept
{
	_triggerNbrPerCollider[col1.Index]++;
	_triggerNbrPerCollider[col2.Index]++;
	if (col1.Index == 0)
	{
		printf("collision: nb = %i\n", _triggerNbrPerCollider[col1.Index]);
	}
	if (col2.Index == 0)
	{
		printf("collision: nb = %i\n", _triggerNbrPerCollider[col2.Index]);
	}
}

void TriggerSample::OnTriggerExit(ColliderRef col1, ColliderRef col2) noexcept
{
	_triggerNbrPerCollider[col1.Index]--;
	_triggerNbrPerCollider[col2.Index]--;
	if (col1.Index == 0)
	{
		printf("sortie: nb = %i\n", _triggerNbrPerCollider[col1.Index]);
	}
	if (col2.Index == 0)
	{
		printf("sortie: nb = %i\n", _triggerNbrPerCollider[col2.Index]);
	}
	////fix de clochard
	//_triggerNbrPerCollider[col1.Index] = 0;
	//_triggerNbrPerCollider[col2.Index] = 0;
}

void TriggerSample::SampleSetUp() noexcept
{
	_world.SetContactListener(this);
	_nbObjects = sphere_NBR + cuboid_NBR + TRIANGLE_NBR;
	_triggerNbrPerCollider.resize(_nbObjects, 0);
	AllGraphicsData.reserve(_nbObjects);
	_bodyRefs.reserve(_nbObjects);
	_colRefs.reserve(_nbObjects);

	//Create spheres
	for (std::size_t i = 0; i < sphere_NBR; ++i)
	{
		const auto sphereBodyRef = _world.CreateBody();
		_bodyRefs.push_back(sphereBodyRef);
		auto& sphereBody = _world.GetBody(sphereBodyRef);

		sphereBody.Velocity = XMVectorScale(XMVectorSet(Random::Range(-1.f, 1.f), Random::Range(-1.f, 1.f), Random::Range(-1.f, 1.f), 0), SPEED);

		sphereBody.Position = { Random::Range(-100.f, 100.f),Random::Range(-100.f, 100.f), Random::Range(-100.f, 100.f) };

		const auto sphereColRef = _world.CreateCollider(sphereBodyRef);
		_colRefs.push_back(sphereColRef);
		auto& sphereCol = _world.GetCollider(sphereColRef);
		sphereCol.Shape = Sphere(XMVectorZero(), sphere_RADIUS);
		sphereCol.BodyPosition = sphereBody.Position;
		sphereCol.IsTrigger = true;

		GraphicsData cgd;
		cgd.Shape = Sphere(XMVectorZero(), sphere_RADIUS) + sphereBody.Position;
		AllGraphicsData.push_back(cgd);
	}

	//Create cuboids
	for (std::size_t i = 0; i < cuboid_NBR; ++i)
	{
		const auto rectBodyRef = _world.CreateBody();
		_bodyRefs.push_back(rectBodyRef);
		auto& rectBody = _world.GetBody(rectBodyRef);

		rectBody.Velocity = XMVectorScale(XMVectorSet(Random::Range(-1.f, 1.f), Random::Range(-1.f, 1.f), Random::Range(-1.f, 1.f), 0), SPEED);

		rectBody.Position = { Random::Range(-100.f, 100.f),Random::Range(-100.f, 100.f),Random::Range(-100.f, 100.f) };

		const auto rectColRef = _world.CreateCollider(rectBodyRef);
		_colRefs.push_back(rectColRef);
		auto& rectCol = _world.GetCollider(rectColRef);
		rectCol.Shape = CuboidF(XMVectorZero(), cuboid_BOUNDS);
		rectCol.BodyPosition = rectBody.Position;
		rectCol.IsTrigger = true;

		GraphicsData rbd;
		rbd.Shape = CuboidF(XMVectorZero(), cuboid_BOUNDS) + rectBody.Position;
		AllGraphicsData.push_back(rbd);
	}
}

void TriggerSample::DrawQuadtree(const BVHNode& node) noexcept
{
	static Color color = { 255,255,255,255 };
	if (node.Children[0] == nullptr)
	{

		_quadTreeGraphicsData.push_back({ CuboidF(node.Bounds), false ,color });

	}
	else
	{
		for (int i = 0; i < node.Children.size(); i++)
		{
			switch (i)
			{
			case 0: color.r = 255; break;
			case 1: color.g = 255; break;
			case 2: color.b = 255; break;
			case 3: color.r = 255; color.g = 255; color.b = 255; break;
			case 4: color.r = 255; color.g = 255; break;
			case 5: color.r = 255; color.b = 255; break;
			case 6: color.g = 255; color.b = 255; break;
			case 7: color.r = 50; color.g = 50; color.b = 50; break;
			default:
				color = { 255,255,255,255 };
				break;

			}
			DrawQuadtree(*node.Children[i]);
		}
	}
	color = { 255,255,255,255 };
}

void TriggerSample::SampleUpdate() noexcept
{
	if (_nbObjects < AllGraphicsData.size())
	{
		AllGraphicsData.erase(AllGraphicsData.begin() + _nbObjects, AllGraphicsData.end());
	}

	for (std::size_t i = 0; i < _colRefs.size(); ++i)
	{
		auto& col = _world.GetCollider(_colRefs[i]);
		auto bounds = col.GetBounds();
		auto& body = _world.GetBody(col.BodyRef);

		if (XMVectorGetX(bounds.MinBound()) <= -100)
		{
			body.Velocity = XMVectorSetX(body.Velocity, Abs(XMVectorGetX(body.Velocity)));
		}
		else if (XMVectorGetX(bounds.MaxBound()) >= 100)
		{
			body.Velocity = XMVectorSetX(body.Velocity, -Abs(XMVectorGetX(body.Velocity)));
		}
		if (XMVectorGetY(bounds.MinBound()) <= -100)
		{
			body.Velocity = XMVectorSetY(body.Velocity, Abs(XMVectorGetY(body.Velocity)));
		}
		else if (XMVectorGetY(bounds.MaxBound()) >= 100)
		{
			body.Velocity = XMVectorSetY(body.Velocity, -Abs(XMVectorGetY(body.Velocity)));
		}
		if (XMVectorGetZ(bounds.MinBound()) <= -100)
		{
			body.Velocity = XMVectorSetZ(body.Velocity, Abs(XMVectorGetZ(body.Velocity)));
		}
		else if (XMVectorGetZ(bounds.MaxBound()) >= 100)
		{
			body.Velocity = XMVectorSetZ(body.Velocity, -Abs(XMVectorGetZ(body.Velocity)));
		}

		auto& shape = _world.GetCollider(_colRefs[i]).Shape;

		switch (shape.index())
		{
		case static_cast<int>(ShapeType::Sphere):
			AllGraphicsData[i].Shape = std::get<SphereF>(shape) + body.Position;
			break;
		case static_cast<int>(ShapeType::Cuboid):
			AllGraphicsData[i].Shape = std::get<CuboidF>(shape) + body.Position;
			break;
		}

		if (_triggerNbrPerCollider[i] > 0)
		{
			if (i == 0)
			{
				AllGraphicsData[i].Color = { 255, 255, 0, 255 };
			}
			else
				AllGraphicsData[i].Color = { 0, 255, 0, 255 };
		}
		else
		{
			if (i == 0)
			{
				AllGraphicsData[i].Color = { 255, 0, 0, 255 };
			}
			else {
				AllGraphicsData[i].Color = { 0, 0, 255, 255 };
			}
		}
	}

	_quadTreeGraphicsData.clear();
	DrawQuadtree(_world.OctTree.Nodes[0]);
	AllGraphicsData.insert(AllGraphicsData.end(), _quadTreeGraphicsData.begin(), _quadTreeGraphicsData.end());
}

void TriggerSample::SampleTearDown() noexcept
{
	_triggerNbrPerCollider.clear();
	_quadTreeGraphicsData.clear();
}
