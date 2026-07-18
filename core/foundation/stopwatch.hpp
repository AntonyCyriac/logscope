/**
 * @file stopwatch.hpp
 * @brief Elapsed-time stopwatch.
 */

#pragma once

#include <chrono>
#include <cstdint>

#include "duration.hpp"

namespace scope::foundation
{

/**
 * @brief Measures elapsed time using a monotonic steady clock.
 */
class Stopwatch
{
  public:
    /**
     * @brief Constructs a stopwatch and starts it immediately.
     */
    Stopwatch() noexcept;

    /**
     * @brief Starts or resumes the stopwatch.
     */
    void start() noexcept;

    /**
     * @brief Stops the stopwatch.
     */
    void stop() noexcept;

    /**
     * @brief Resets elapsed time to zero.
     *
     * If the stopwatch is running, timing restarts from zero.
     */
    void reset() noexcept;

    /**
     * @brief Determines whether the stopwatch is running.
     */
    [[nodiscard]] bool isRunning() const noexcept;

    /**
     * @brief Returns the elapsed time.
     */
    [[nodiscard]] Duration elapsed() const;

  private:
    using SteadyClock = std::chrono::steady_clock;

    [[nodiscard]] std::int64_t currentElapsedNanoseconds() const noexcept;

    bool m_running{false};

    SteadyClock::time_point m_start{};

    std::int64_t m_elapsedNanoseconds{0};
};

} // namespace scope::foundation
