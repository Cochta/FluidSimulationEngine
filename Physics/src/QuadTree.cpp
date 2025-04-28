#include "QuadTree.h"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#include <TracyC.h>
#endif 

BVHNode::BVHNode(Allocator& alloc) noexcept : ColliderRefAabbs(StandardAllocator<ColliderRefPair>{alloc})
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	ColliderRefAabbs.reserve(MAX_COL_NBR);
}

BVHNode::BVHNode(const CuboidF& bounds, Allocator& alloc) noexcept : ColliderRefAabbs(StandardAllocator<ColliderRefPair>{ alloc }), Bounds(bounds)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	ColliderRefAabbs.reserve(MAX_COL_NBR);
}

OctTree::OctTree(Allocator& alloc) noexcept : _alloc(alloc), Nodes{ StandardAllocator<BVHNode>{alloc} }
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	std::size_t result = 0;
	for (size_t i = 0; i <= MAX_DEPTH; i++)
	{
		result += Pow(SUBDIVIDE_NBR, i);
	}
	Nodes.resize(result, BVHNode(_alloc));

}

void OctTree::SubdivideNode(BVHNode& node, float sphRadius) noexcept
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	// Compute half the size of the bounding box
	const XMVECTOR halfSize = XMVectorScale(XMVectorSubtract(node.Bounds.MaxBound(), node.Bounds.MinBound()), 0.5f);
	const XMVECTOR minBound = node.Bounds.MinBound();
	sphRadius = 0;
	const XMVECTOR expand = XMVectorReplicate(sphRadius);

	// Precompute offsets for all 8 child nodes
	const XMVECTOR offsets[SUBDIVIDE_NBR] = {
		XMVectorSet(0, 0, 0, 0),                         // Bottom-left-front
		XMVectorSet(0, 0, XMVectorGetZ(halfSize), 0),    // Bottom-left-back
		XMVectorSet(0, XMVectorGetY(halfSize), 0, 0),    // Bottom-right-front
		XMVectorSet(0, XMVectorGetY(halfSize), XMVectorGetZ(halfSize), 0), // Bottom-right-back
		XMVectorSet(XMVectorGetX(halfSize), 0, 0, 0),    // Top-left-front
		XMVectorSet(XMVectorGetX(halfSize), 0, XMVectorGetZ(halfSize), 0), // Top-left-back
		XMVectorSet(XMVectorGetX(halfSize), XMVectorGetY(halfSize), 0, 0), // Top-right-front
		XMVectorSet(XMVectorGetX(halfSize), XMVectorGetY(halfSize), XMVectorGetZ(halfSize), 0) // Top-right-back
	};

	// Create 8 child nodes
	for (int i = 0; i < SUBDIVIDE_NBR; ++i)
	{
		node.Children[i] = &Nodes[_nodeIndex + i];
		const XMVECTOR childMin = XMVectorSubtract(XMVectorAdd(minBound, offsets[i]), expand);
		const XMVECTOR childMax = XMVectorAdd(XMVectorAdd(XMVectorAdd(minBound, offsets[i]), halfSize), expand);
		node.Children[i]->Bounds = { childMin, childMax };
		node.Children[i]->Depth = node.Depth + 1;
	}

	// Increment node index for the next subdivision
	_nodeIndex += 8;
}

void OctTree::Insert(BVHNode& node, const ColliderRefAabb& colliderRefAabb) noexcept
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	if (node.Children[0] != nullptr)
	{
		for (const auto& child : node.Children)
		{
			if (Intersect(colliderRefAabb.Aabb, child->Bounds))
			{
				Insert(*child, colliderRefAabb);
			}
		}
	}
	else if (node.ColliderRefAabbs.size() >= MAX_COL_NBR && node.Depth < MAX_DEPTH)
	{
		SubdivideNode(node, SPH::SmoothingRadius);
		node.ColliderRefAabbs.push_back(colliderRefAabb);
		for (const auto& child : node.Children)
		{
			for (auto& col : node.ColliderRefAabbs)
			{
				if (Intersect(col.Aabb, child->Bounds))
				{
					Insert(*child, col);
				}
			}
		}
		node.ColliderRefAabbs.clear();
		//node.ColliderRefAabbs.shrink_to_fit();
	}
	else
	{
		node.ColliderRefAabbs.push_back(colliderRefAabb);
		//printf("node depth: %i, nb col: %i", node.Depth, node.ColliderRefAabbs.size());
	}
}

void OctTree::SetUpRoot(const CuboidF& bounds) noexcept
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	for (auto& node : Nodes)
	{
		node.ColliderRefAabbs.clear();
		std::fill(node.Children.begin(), node.Children.end(), nullptr);
	}
	Nodes[0].Bounds = bounds;

	_nodeIndex = 1;
}
