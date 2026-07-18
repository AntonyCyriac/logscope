/**
 * @file time.hpp
 * @brief Time-of-day value type.
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "result.hpp"

namespace scope::foundation
{

/**
 * @brief Represents a time of day with nanosecond precision.
 *
 * Valid range: 00:00:00.000000000 through 23:59:59.999999999.
 */
class Time
{
  public:
    /**
     * @brief Constructs midnight (00:00:00).
     */
    constexpr Time() noexcept;

    /**
     * @brief Constructs a time of day.
     *
     * @param hour Hour of day (0–23).
     * @param minute Minute (0–59).
     * @param second Second (0–59).
     * @param nanosecond Nanosecond (0–999999999).
     */
    constexpr Time(int hour, int minute, int second, int nanosecond = 0) noexcept;

    /**
     * @brief Parses a time string (HH:MM:SS or HH:MM:SS.nnnnnnnnn).
     *
     * @param value Time string.
     * @return Parsed time or error.
     */
    static Result<Time> parse(std::string_view value);

    [[nodiscard]] constexpr int hour() const noexcept;

    [[nodiscard]] constexpr int minute() const noexcept;

    [[nodiscard]] constexpr int second() const noexcept;

    [[nodiscard]] constexpr int nanosecond() const noexcept;

    /**
     * @brief Returns canonical time string (HH:MM:SS).
     */
    std::string toString() const;

    friend constexpr bool operator==(const Time& lhs, const Time& rhs) noexcept;

    friend constexpr bool operator!=(const Time& lhs, const Time& rhs) noexcept;

    friend constexpr bool operator<(const Time& lhs, const Time& rhs) noexcept;

  private:
    int m_hour{0};

    int m_minute{0};

    int m_second{0};

    int m_nanosecond{0};
};

} // namespace scope::foundation

#include "time.inl"
