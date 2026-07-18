/**
 * @file time.inl
 * @brief Implements constexpr members of Time.
 */

#pragma once

namespace scope::foundation
{

constexpr Time::Time() noexcept = default;

constexpr Time::Time(int hour, int minute, int second, int nanosecond) noexcept
    : m_hour(hour)
    , m_minute(minute)
    , m_second(second)
    , m_nanosecond(nanosecond)
{
}

constexpr int Time::hour() const noexcept
{
    return m_hour;
}

constexpr int Time::minute() const noexcept
{
    return m_minute;
}

constexpr int Time::second() const noexcept
{
    return m_second;
}

constexpr int Time::nanosecond() const noexcept
{
    return m_nanosecond;
}

constexpr bool operator==(const Time& lhs, const Time& rhs) noexcept
{
    return lhs.hour() == rhs.hour() && lhs.minute() == rhs.minute() && lhs.second() == rhs.second() &&
           lhs.nanosecond() == rhs.nanosecond();
}

constexpr bool operator!=(const Time& lhs, const Time& rhs) noexcept
{
    return !(lhs == rhs);
}

constexpr bool operator<(const Time& lhs, const Time& rhs) noexcept
{
    if (lhs.hour() != rhs.hour())
    {
        return lhs.hour() < rhs.hour();
    }

    if (lhs.minute() != rhs.minute())
    {
        return lhs.minute() < rhs.minute();
    }

    if (lhs.second() != rhs.second())
    {
        return lhs.second() < rhs.second();
    }

    return lhs.nanosecond() < rhs.nanosecond();
}

} // namespace scope::foundation
