#include <chrono>

/**
 * @brief A timer for measuring time intervals and calculating delta time.
 * @note This class is used to measure time intervals and calculate the time difference between ticks.
 */
class Timer
{
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> _startTime; // The starting time point.

public:
    float DeltaTime{0.f}; // The time elapsed between ticks.

    /**
     * @brief Set up the timer by recording the initial time point.
     */
    void SetUp() noexcept;

    /**
     * @brief Update the timer by calculating the delta time since the last tick.
     */
    void Tick() noexcept;
};