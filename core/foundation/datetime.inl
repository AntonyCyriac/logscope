/**
 * @file datetime.inl
 * @brief Implements constexpr members of DateTime.
 */

#pragma once

namespace scope::foundation
{

constexpr DateTime::DateTime() noexcept = default;

constexpr DateTime::DateTime(Date date, Time time) noexcept
    : m_date(date)
    , m_time(time)
{
}

constexpr const Date& DateTime::date() const noexcept
{
    return m_date;
}

constexpr const Time& DateTime::time() const noexcept
{
    return m_time;
}

constexpr bool operator==(const DateTime& lhs, const DateTime& rhs) noexcept
{
    return lhs.date() == rhs.date() && lhs.time() == rhs.time();
}

constexpr bool operator!=(const DateTime& lhs, const DateTime& rhs) noexcept
{
    return !(lhs == rhs);
}

constexpr bool operator<(const DateTime& lhs, const DateTime& rhs) noexcept
{
    if (lhs.date() != rhs.date())
    {
        return lhs.date() < rhs.date();
    }

    return lhs.time() < rhs.time();
}

} // namespace scope::foundation
