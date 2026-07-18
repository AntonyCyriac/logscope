/**
 * @file datetime.hpp
 * @brief Combined date and time value type.
 */

#pragma once

#include <string>
#include <string_view>

#include "date.hpp"
#include "result.hpp"
#include "time.hpp"

namespace scope::foundation
{

/**
 * @brief Represents a combined calendar date and time of day.
 */
class DateTime
{
  public:
    /**
     * @brief Constructs the epoch (1970-01-01 00:00:00).
     */
    constexpr DateTime() noexcept;

    /**
     * @brief Constructs a date-time from components.
     *
     * @param date Calendar date.
     * @param time Time of day.
     */
    constexpr DateTime(Date date, Time time) noexcept;

    /**
     * @brief Parses an ISO-like string (YYYY-MM-DDTHH:MM:SS).
     *
     * @param value Date-time string.
     * @return Parsed date-time or error.
     */
    static Result<DateTime> parse(std::string_view value);

    [[nodiscard]] constexpr const Date& date() const noexcept;

    [[nodiscard]] constexpr const Time& time() const noexcept;

    /**
     * @brief Returns canonical date-time string.
     */
    std::string toString() const;

    friend constexpr bool operator==(const DateTime& lhs, const DateTime& rhs) noexcept;

    friend constexpr bool operator!=(const DateTime& lhs, const DateTime& rhs) noexcept;

    friend constexpr bool operator<(const DateTime& lhs, const DateTime& rhs) noexcept;

  private:
    Date m_date{};

    Time m_time{};
};

} // namespace scope::foundation

#include "datetime.inl"
