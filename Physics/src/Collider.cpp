#include "Collider.h"

CuboidF Collider::GetBounds() const noexcept
{
	switch (Shape.index())
	{
	case static_cast<int>(ShapeType::Sphere):
	{
		auto sphere = std::get<SphereF>(Shape);
		return CuboidF::FromCenter(sphere.Center(), { sphere.Radius(), sphere.Radius(), sphere.Radius()}) + BodyPosition;
	}
	case static_cast<int>(ShapeType::Cuboid):
	{
		return std::get<CuboidF>(Shape) + BodyPosition;
	}
	}
	return { XMVectorZero(), XMVectorZero() };
}

bool ColliderRefPair::operator==(const ColliderRefPair& other) const
{
	return (ColRefA == other.ColRefA && ColRefB == other.ColRefB) ||
		(ColRefA == other.ColRefB && ColRefB == other.ColRefA);
}


std::size_t ColliderRefPairHash::operator()(const ColliderRefPair& pair) const
{
	const std::size_t hashA = std::hash<size_t>{}(pair.ColRefA.Index);
	const std::size_t hashB = std::hash<size_t>{}(pair.ColRefB.Index);

	// XOR for the hash
	return hashA ^ hashB;
}
