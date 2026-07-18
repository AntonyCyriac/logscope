/**
 * @file date.hpp
 * @brief Calendar date value type.
 */

#pragma once

#include <string>
#include <string_view>

#include "result.hpp"

namespace scope::foundation
{

/**
 * @brief Represents a calendar date (year-month-day).
 */
class Date
{
  public:
    /**
     * @brief Constructs the epoch date (1970-01-01).
     */
    constexpr Date() noexcept;

    /**
     * @brief Constructs a calendar date.
     *
     * @param year Four-digit year.
     * @param month Month (1–12).
     * @param day Day (1–31).
     */
    constexpr Date(int year, int month, int day) noexcept;

    /**
     * @brief Parses an ISO date string (YYYY-MM-DD).
     *
     * @param value Date string.
     * @return Parsed date or error.
     */
    static Result<Date> parse(std::string_view value);

    [[nodiscard]] constexpr int year() const noexcept;

    [[nodiscard]] constexpr int month() const noexcept;

    [[nodiscard]] constexpr int day() const noexcept;

    /**
     * @brief Returns canonical date string (YYYY-MM-DD).
     */
    std::string toString() const;

    friend constexpr bool operator==(const Date& lhs, const Date& rhs) noexcept;

    friend constexpr bool operator!=(const Date& lhs, const Date& rhs) noexcept;

    friend constexpr bool operator<(const Date& lhs, const Date& rhs) noexcept;

  private:
    int m_year{1970};

    int m_month{1};

    int m_day{1};
};

} // namespace scope::foundation

#include "date.inl"
