#include "PhysicsSample.h"

void PhysicsSample::SetUp() noexcept
{
	//_timer.SetUp();
	_world.SetUp();
	SampleSetUp();
}

void PhysicsSample::Update() noexcept
{
	SampleUpdate();
	//_timer.Tick();
	if (_isActive)
	{
		_world.Update(1.f / 60.f);
	}

	_mouseLeftReleased = false;
	_mouseRightReleased = false;
}

void PhysicsSample::TearDown() noexcept
{
	SampleTearDown();
	_bodyRefs.clear();
	AllGraphicsData.clear();
	_colRefs.clear();
	_world.TearDown();
}

void PhysicsSample::GetMousePos(const XMVECTOR mousePos) noexcept
{
	_mousePos = mousePos;
}

void PhysicsSample::OnLeftClick() noexcept
{
	_mouseLeftReleased = true;
}

void PhysicsSample::OnRightClick() noexcept
{
	_mouseRightReleased = true;
}

void PhysicsSample::OnStop() noexcept
{
	_isActive = !_isActive;
}
