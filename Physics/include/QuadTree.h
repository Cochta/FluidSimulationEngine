#pragma once

#include "Collider.h"
#include "Allocators.h"
#include "UniquePtr.h"
#include "SPH.h"

#include <memory>
#include <array>
#include <unordered_map>

static constexpr int MAX_COL_NBR = 16; /**< Maximum number of colliders in a quadtree node. */
static constexpr int MAX_DEPTH = 4; /**< Maximum depth of the quadtree. */
static constexpr int SUBDIVIDE_NBR = 8; /**<Number of subdivisions>*/

/**
 * @brief Structure representing a bounding box (AABB) associated with a collider reference.
 */
struct ColliderRefAabb
{
	const CuboidF Aabb;    /**< The bounding box (AABB). */
	const ColliderRef ColRef;       /**< The reference to a collider. */
};

/**
 * @brief Class representing a node in a quadtree data structure for collision detection.
 */
class BVHNode
{
public:
	CustomlyAllocatedVector<ColliderRefAabb> ColliderRefAabbs;  /**< Vector of collider references with AABBs. */
	CuboidF Bounds{ XMVectorZero(), XMVectorZero() }; /**< The bounds of the quadtree node. */
	std::array<BVHNode*, SUBDIVIDE_NBR> Children{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }; /**< Array of child nodes. */
	int Depth = 0; /**< The depth of the node in the quadtree.*/

	/**
	 * @brief Constructor for BVHNode, allocating memory using a specified allocator.
	 * @param alloc The allocator for memory allocation.
	 */
	BVHNode(Allocator& alloc) noexcept;

	/**
	 * @brief Constructor for BVHNode with a specified bounds, allocating memory using a specified allocator.
	 * @param bounds The bounds of the quadtree node.
	 * @param alloc The allocator for memory allocation.
	 */
	explicit BVHNode(const CuboidF& bounds, Allocator& alloc) noexcept;
};

/**
 * @brief Class representing a quadtree data structure for collision detection.
 */
class OctTree {
public:
	CustomlyAllocatedVector<BVHNode> Nodes; /**< Vector of quadtree nodes. */

private:

	int _nodeIndex = 1; /**< The index of the current node in the quadtree. */
	Allocator& _alloc; /**< The allocator for memory allocation.*/

public:
	/**
	 * @brief Constructor for OctTree, allocating memory using a specified allocator.
	 * @param alloc The allocator for memory allocation.
	 */
	OctTree(Allocator& alloc) noexcept;

	/**
	 * @brief Set up the root of the quadtree with the specified bounds.
	 * @param bounds The bounds of the root node.
	 */
	void SetUpRoot(const CuboidF& bounds) noexcept;

	/**
	 * @brief Insert a collider reference with an associated AABB into the quadtree.
	 * @param node The node to insert into.
	 * @param colliderRefAabb The collider reference with an AABB to insert.
	 */
	void Insert(BVHNode& node, const ColliderRefAabb& colliderRefAabb) noexcept;
private:
	/**
	 * @brief Subdivide a quadtree node into smaller child nodes.
	 * @param node The node to subdivide.
	 */
	void SubdivideNode(BVHNode& node, float sphRadius = 0) noexcept;
};
struct XMINT3Equal {
	bool operator()(const XMINT3& a, const XMINT3& b) const {
		return a.x == b.x && a.y == b.y && a.z == b.z;
	}
};

// Convertit un vecteur en indices de grille
inline XMINT3 getGridIndex(const XMVECTOR& pos) {
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	return XMINT3(
		static_cast<int>(XMVectorGetX(pos) / SPH::SmoothingRadius),
		static_cast<int>(XMVectorGetY(pos) / SPH::SmoothingRadius),
		static_cast<int>(XMVectorGetZ(pos) / SPH::SmoothingRadius)
	);
}

// Fonction de hachage pour la grille
struct GridHash {
	size_t operator()(const XMINT3& cell) const {
		return cell.x ^ cell.y ^ cell.z;
	}
};
constexpr XMINT3 offsets[] = {
{-1, -1, -1}, {-1, -1, 0}, {-1, -1, 1}, {-1, 0, -1}, {-1, 0, 0}, {-1, 0, 1}, {-1, 1, -1}, {-1, 1, 0}, {-1, 1, 1},
{0, -1, -1}, {0, -1, 0}, {0, -1, 1}, {0, 0, -1}, {0, 0, 0}, {0, 0, 1}, {0, 1, -1}, {0, 1, 0}, {0, 1, 1},
{1, -1, -1}, {1, -1, 0}, {1, -1, 1}, {1, 0, -1}, {1, 0, 0}, {1, 0, 1}, {1, 1, -1}, {1, 1, 0}, {1, 1, 1}
};

// Grille pour SPH
struct SpatialHashGrid {
	std::unordered_map<XMINT3, std::vector<BodyRef>, GridHash, XMINT3Equal> grid;

	void clear() {
		grid.clear();
	}

	// Ajoute une particule à la grille
	void insertParticle(const BodyRef& ref, const XMVECTOR& position) {
		XMINT3 cell = getGridIndex(position);
		grid[cell].push_back(ref);
	}
	// Trouve les voisins dans un rayon h
	std::vector<BodyRef> findNeighbors(const XMVECTOR& position) {
		std::vector<BodyRef> neighbors;
		neighbors.reserve(50);  // Réserve de la mémoire pour éviter les réallocations fréquentes.

		XMINT3 cell = getGridIndex(position);

		for (const auto& offset : offsets) {
			XMINT3 neighborCell = XMINT3(cell.x + offset.x, cell.y + offset.y, cell.z + offset.z);
			auto it = grid.find(neighborCell);
			if (it != grid.end()) {
				const std::vector<BodyRef>* neighborList = &it->second; // Utilisation d'un pointeur
				neighbors.insert(neighbors.end(), neighborList->begin(), neighborList->end());
			}
		}
		return neighbors;
	}
};