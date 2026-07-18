/**
 * @file clock.cpp
 * @brief Clock implementation.
 */

#include "clock.hpp"

#include <chrono>

namespace scope::foundation
{

Timestamp Clock::now() noexcept
{
    const auto now = std::chrono::system_clock::now();
    const auto nanoseconds =
        std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();

    return Timestamp(nanoseconds);
}

} // namespace scope::foundation
