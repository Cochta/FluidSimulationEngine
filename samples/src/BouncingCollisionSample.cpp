#include "BouncingCollisionSample.h"

std::string BouncingCollisionSample::GetName() noexcept { return "Collision"; }

std::string BouncingCollisionSample::GetDescription() noexcept {
  return "Randomly generated objects, they change color on contact with "
         "another object, the collision detection uses a QuadTree. ";
}

void BouncingCollisionSample::OnCollisionEnter(ColliderRef col1,
                                               ColliderRef col2) noexcept {
  Color color = {Random::Range(0, 255), Random::Range(0, 255),
                 Random::Range(0, 255), 255};
  AllGraphicsData[col1.Index].Color = color;
  AllGraphicsData[col2.Index].Color = color;
}

void BouncingCollisionSample::OnCollisionExit(ColliderRef col1,
                                              ColliderRef col2) noexcept {}

void BouncingCollisionSample::SampleSetUp() noexcept {
  _world.SetContactListener(this);
  _nbObjects = sphere_NBR + cuboid_NBR;
  _collisionNbrPerCollider.resize(_nbObjects, 0);
  AllGraphicsData.reserve(_nbObjects);
  _bodyRefs.reserve(_nbObjects);
  _colRefs.reserve(_nbObjects);

  // Create spheres
  for (std::size_t i = 0; i < sphere_NBR; ++i) {
    auto bodyRef1 = _world.CreateBody();
    _bodyRefs.push_back(bodyRef1);
    auto& body1 = _world.GetBody(bodyRef1);

    body1.Mass = 1;

    body1.Velocity = XMVectorScale(
        XMVectorSet(Random::Range(-1.f, 1.f), Random::Range(-1.f, 1.f),
                    Random::Range(-1.f, 1.f), 0),
        SPEED);

    body1.Position = {Random::Range(-100.f, 100.f),
                      Random::Range(-100.f, 100.f),
                      Random::Range(-100.f, 100.f)};

    auto colRef1 = _world.CreateCollider(bodyRef1);
    _colRefs.push_back(colRef1);
    auto& col1 = _world.GetCollider(colRef1);
    col1.Shape = Sphere(XMVectorZero(), sphere_RADIUS);
    col1.BodyPosition = body1.Position;

    GraphicsData bd;
    bd.Shape = Sphere(XMVectorZero(), sphere_RADIUS) + body1.Position;
    AllGraphicsData.push_back(bd);
  }

  // Create cuboids
  for (std::size_t i = 0; i < cuboid_NBR; ++i) {
    auto bodyRef1 = _world.CreateBody();
    _bodyRefs.push_back(bodyRef1);
    auto& body1 = _world.GetBody(bodyRef1);

    body1.Velocity = XMVectorScale(
        XMVectorSet(Random::Range(-1.f, 1.f), Random::Range(-1.f, 1.f),
                    Random::Range(-1.f, 1.f), 0),
        SPEED);

    body1.Position = {Random::Range(-100.f, 100.f),
                      Random::Range(-100.f, 100.f),
                      Random::Range(-100.f, 100.f)};

    auto colRef1 = _world.CreateCollider(bodyRef1);
    _colRefs.push_back(colRef1);
    auto& col1 = _world.GetCollider(colRef1);
    col1.Shape = CuboidF(XMVectorZero(), cuboid_BOUNDS);
    col1.BodyPosition = body1.Position;

    GraphicsData bd;
    bd.Shape = CuboidF(XMVectorZero(), cuboid_BOUNDS) + body1.Position;
    AllGraphicsData.push_back(bd);
  }
}

void BouncingCollisionSample::DrawQuadtree(const BVHNode& node) noexcept {
  if (node.Children[0] == nullptr) {
    _quadTreeGraphicsData.push_back({CuboidF(node.Bounds), false});
  } else {
    for (int i = 0; i < 8; i++) {
      DrawQuadtree(*node.Children[i]);
    }
  }
}

void BouncingCollisionSample::SampleUpdate() noexcept {
  if (_nbObjects < AllGraphicsData.size()) {
    AllGraphicsData.erase(AllGraphicsData.begin() + _nbObjects,
                          AllGraphicsData.end());
  }

  for (std::size_t i = 0; i < _colRefs.size(); ++i) {
    auto& col = _world.GetCollider(_colRefs[i]);
    auto bounds = col.GetBounds();
    auto& body = _world.GetBody(col.BodyRef);

    if (XMVectorGetX(bounds.MinBound()) <= -100) {
      body.Velocity =
          XMVectorSetX(body.Velocity, Abs(XMVectorGetX(body.Velocity)));
    } else if (XMVectorGetX(bounds.MaxBound()) >= 100) {
      body.Velocity =
          XMVectorSetX(body.Velocity, -Abs(XMVectorGetX(body.Velocity)));
    }
    if (XMVectorGetY(bounds.MinBound()) <= -100) {
      body.Velocity =
          XMVectorSetY(body.Velocity, Abs(XMVectorGetY(body.Velocity)));
    } else if (XMVectorGetY(bounds.MaxBound()) >= 100) {
      body.Velocity =
          XMVectorSetY(body.Velocity, -Abs(XMVectorGetY(body.Velocity)));
    }
    if (XMVectorGetZ(bounds.MinBound()) <= -100) {
      body.Velocity =
          XMVectorSetZ(body.Velocity, Abs(XMVectorGetZ(body.Velocity)));
    } else if (XMVectorGetZ(bounds.MaxBound()) >= 100) {
      body.Velocity =
          XMVectorSetZ(body.Velocity, -Abs(XMVectorGetZ(body.Velocity)));
    }

    auto& shape = _world.GetCollider(_colRefs[i]).Shape;

    switch (shape.index()) {
      case static_cast<int>(ShapeType::Sphere):
        AllGraphicsData[i].Shape = std::get<SphereF>(shape) + col.BodyPosition;
        break;
      case static_cast<int>(ShapeType::Cuboid):
        AllGraphicsData[i].Shape = std::get<CuboidF>(shape) + col.BodyPosition;
        break;
    }
    if (i == 0) {
                  printf("speed: x=%f, y=%f,z=%f, all=%f \n",
                         XMVectorGetX(body.Velocity), 
                         XMVectorGetY(body.Velocity),
                         XMVectorGetZ(body.Velocity),
             Abs(XMVectorGetX(body.Velocity))+ Abs(XMVectorGetY(body.Velocity))+ Abs(XMVectorGetZ(body.Velocity)));
    }
  }

  _quadTreeGraphicsData.clear();
  DrawQuadtree(_world.OctTree.Nodes[0]);
  AllGraphicsData.insert(AllGraphicsData.end(), _quadTreeGraphicsData.begin(),
                         _quadTreeGraphicsData.end());
}

void BouncingCollisionSample::SampleTearDown() noexcept {
  _collisionNbrPerCollider.clear();
  _quadTreeGraphicsData.clear();
}
