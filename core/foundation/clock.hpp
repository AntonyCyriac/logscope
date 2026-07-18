/**
 * @file clock.hpp
 * @brief Wall-clock time source.
 */

#pragma once

#include "timestamp.hpp"

namespace scope::foundation
{

/**
 * @brief Provides access to the system wall clock.
 */
class Clock
{
  public:
    /**
     * @brief Returns the current UTC timestamp from the system clock.
     */
    [[nodiscard]] static Timestamp now() noexcept;
};

} // namespace scope::foundation
