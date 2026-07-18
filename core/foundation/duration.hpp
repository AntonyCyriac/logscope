/**
 * @file duration.hpp
 * @brief Duration value type.
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "result.hpp"

namespace scope::foundation
{

/**
 * @brief Represents a non-negative time interval with nanosecond precision.
 */
class Duration
{
  public:
    /**
     * @brief Constructs a zero duration.
     */
    constexpr Duration() noexcept;

    /**
     * @brief Constructs a duration from components.
     *
     * @param hours Hour component (>= 0).
     * @param minutes Minute component (0–59).
     * @param seconds Second component (0–59).
     * @param nanoseconds Nanosecond component (0–999999999).
     */
    constexpr Duration(int hours, int minutes, int seconds, int nanoseconds = 0) noexcept;

    /**
     * @brief Constructs a duration from a total number of nanoseconds.
     *
     * @param nanoseconds Total nanoseconds (>= 0).
     * @return Duration or error if the value is negative.
     */
    [[nodiscard]] static Result<Duration> fromNanoseconds(std::int64_t nanoseconds);

    /**
     * @brief Constructs a duration from a total number of seconds.
     *
     * @param seconds Total seconds (>= 0).
     * @return Duration or error if the value is negative.
     */
    [[nodiscard]] static Result<Duration> fromSeconds(std::int64_t seconds);

    /**
     * @brief Parses a duration string (H+:MM:SS or H+:MM:SS.nnnnnnnnn).
     *
     * @param value Duration string.
     * @return Parsed duration or error.
     */
    [[nodiscard]] static Result<Duration> parse(std::string_view value);

    [[nodiscard]] constexpr int hours() const noexcept;

    [[nodiscard]] constexpr int minutes() const noexcept;

    [[nodiscard]] constexpr int seconds() const noexcept;

    [[nodiscard]] constexpr int nanoseconds() const noexcept;

    /**
     * @brief Returns the total duration in nanoseconds.
     */
    [[nodiscard]] constexpr std::int64_t totalNanoseconds() const noexcept;

    /**
     * @brief Returns canonical duration string (H:MM:SS).
     */
    [[nodiscard]] std::string toString() const;

    friend constexpr bool operator==(const Duration& lhs, const Duration& rhs) noexcept;

    friend constexpr bool operator!=(const Duration& lhs, const Duration& rhs) noexcept;

    friend constexpr bool operator<(const Duration& lhs, const Duration& rhs) noexcept;

  private:
    constexpr explicit Duration(std::int64_t nanoseconds) noexcept;

    std::int64_t m_nanoseconds{0};
};

} // namespace scope::foundation

#include "duration.inl"
