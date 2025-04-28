#pragma once

#include <iostream>
#include "PhysicsSample.h"
#include "Random.h"
#include "Utility.h"

class BouncingCollisionSample : public PhysicsSample, public ContactListener
{
private:
	std::vector<GraphicsData> _quadTreeGraphicsData;
	std::vector<int> _collisionNbrPerCollider;

	static constexpr XMVECTOR cuboid_BOUNDS = { Metrics::MetersToPixels(0.15f), Metrics::MetersToPixels(0.15f), Metrics::MetersToPixels(0.15f) };
	static constexpr float sphere_RADIUS = Metrics::MetersToPixels(0.1f);
	static constexpr float SPEED = 100;

	static constexpr std::size_t sphere_NBR = 20;
	static constexpr std::size_t cuboid_NBR = 20;

	std::size_t _nbObjects{};

public:
	std::string GetName() noexcept override;
	std::string GetDescription() noexcept override;

	void OnTriggerEnter(ColliderRef col1, ColliderRef col2) noexcept override {};

	void OnTriggerExit(ColliderRef col1, ColliderRef col2) noexcept override {};

	void OnCollisionEnter(ColliderRef col1, ColliderRef col2) noexcept override;

	void OnCollisionExit(ColliderRef col1, ColliderRef col2) noexcept override;

protected:
	void SampleSetUp() noexcept override;

	void SampleUpdate() noexcept override;

	void SampleTearDown() noexcept override;

private:
	void DrawQuadtree(const BVHNode& node) noexcept;

};
