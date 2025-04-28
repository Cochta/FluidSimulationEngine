#include "Timer.h"

void Timer::SetUp() noexcept
{
    _startTime = std::chrono::high_resolution_clock::now();
}

void Timer::Tick() noexcept
{
    auto currentTime = std::chrono::high_resolution_clock::now();
    DeltaTime = std::chrono::duration<float>(currentTime - _startTime).count();
    _startTime = currentTime;
}