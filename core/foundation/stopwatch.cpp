/**
 * @file stopwatch.cpp
 * @brief Stopwatch implementation.
 */

#include "stopwatch.hpp"

namespace scope::foundation
{

Stopwatch::Stopwatch() noexcept
{
    start();
}

void Stopwatch::start() noexcept
{
    if (!m_running)
    {
        m_start = SteadyClock::now();
        m_running = true;
    }
}

void Stopwatch::stop() noexcept
{
    if (m_running)
    {
        m_elapsedNanoseconds = currentElapsedNanoseconds();
        m_running = false;
    }
}

void Stopwatch::reset() noexcept
{
    m_elapsedNanoseconds = 0;

    if (m_running)
    {
        m_start = SteadyClock::now();
    }
}

bool Stopwatch::isRunning() const noexcept
{
    return m_running;
}

Duration Stopwatch::elapsed() const
{
    return *Duration::fromNanoseconds(currentElapsedNanoseconds());
}

std::int64_t Stopwatch::currentElapsedNanoseconds() const noexcept
{
    if (!m_running)
    {
        return m_elapsedNanoseconds;
    }

    const auto delta = SteadyClock::now() - m_start;

    return m_elapsedNanoseconds +
           std::chrono::duration_cast<std::chrono::nanoseconds>(delta).count();
}

} // namespace scope::foundation
