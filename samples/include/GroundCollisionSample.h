#pragma once

#include <iostream>
#include "PhysicsSample.h"
#include "Random.h"


static constexpr float SPEED = -500;

class GroundCollisionSample : public PhysicsSample, public ContactListener
{
private:


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
	void CreateBall(XMVECTOR position) noexcept;
	void CreateRect(XMVECTOR position) noexcept;

};
