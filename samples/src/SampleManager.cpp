#include "SampleManager.h"

void SampleManager::SetUp() noexcept
{
	//_samples.push_back(std::make_unique<DefaultSample>());
	//_samples.push_back(std::make_unique<StarSystemSample>());
	//_samples.push_back(std::make_unique<TriggerSample>());
	//_samples.push_back(std::make_unique<BouncingCollisionSample>());
	//_samples.push_back(std::make_unique<GroundCollisionSample>());
	_samples.push_back(std::make_unique<WaterBathSample>());

	_samples[_sampleIdx]->SetUp();
}

void SampleManager::UpdateSample() const noexcept
{
	_samples[_sampleIdx]->Update();
}

void SampleManager::ChangeSample(int idx) noexcept
{
	_samples[_sampleIdx]->TearDown();
	_sampleIdx = idx;
	_samples[_sampleIdx]->SetUp();
}

void SampleManager::NextSample() noexcept
{
	_samples[_sampleIdx]->TearDown();
	if (_sampleIdx >= _samples.size() - 1)
		_sampleIdx = 1;
	else
		_sampleIdx++;
	_samples[_sampleIdx]->SetUp();
}

void SampleManager::PreviousSample() noexcept
{
	_samples[_sampleIdx]->TearDown();
	if (_sampleIdx <= 1)
		_sampleIdx = _samples.size() - 1;
	else
		_sampleIdx--;
	_samples[_sampleIdx]->SetUp();
}

void SampleManager::RegenerateSample() const noexcept
{
	_samples[_sampleIdx]->TearDown();
	_samples[_sampleIdx]->SetUp();
	printf("\n");
}

std::vector<GraphicsData>& SampleManager::GetSampleData() const noexcept
{
	return _samples[_sampleIdx]->AllGraphicsData;
}

void SampleManager::GiveMousePositionToSample(const XMVECTOR mousePosition) const noexcept
{
	_samples[_sampleIdx]->GetMousePos(mousePosition);
}

void SampleManager::GiveLeftMouseClickToSample() const noexcept
{
	_samples[_sampleIdx]->OnLeftClick();
}

void SampleManager::GiveRightMouseClickToSample() const noexcept
{
	_samples[_sampleIdx]->OnRightClick();
}

void SampleManager::StopSample() const noexcept
{
	_samples[_sampleIdx]->OnStop();
}
