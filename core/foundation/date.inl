/**
 * @file date.inl
 * @brief Implements constexpr members of Date.
 */

#pragma once

namespace scope::foundation
{

constexpr Date::Date() noexcept = default;

constexpr Date::Date(int year, int month, int day) noexcept
    : m_year(year)
    , m_month(month)
    , m_day(day)
{
}

constexpr int Date::year() const noexcept
{
    return m_year;
}

constexpr int Date::month() const noexcept
{
    return m_month;
}

constexpr int Date::day() const noexcept
{
    return m_day;
}

constexpr bool operator==(const Date& lhs, const Date& rhs) noexcept
{
    return lhs.year() == rhs.year() && lhs.month() == rhs.month() && lhs.day() == rhs.day();
}

constexpr bool operator!=(const Date& lhs, const Date& rhs) noexcept
{
    return !(lhs == rhs);
}

constexpr bool operator<(const Date& lhs, const Date& rhs) noexcept
{
    if (lhs.year() != rhs.year())
    {
        return lhs.year() < rhs.year();
    }

    if (lhs.month() != rhs.month())
    {
        return lhs.month() < rhs.month();
    }

    return lhs.day() < rhs.day();
}

} // namespace scope::foundation
