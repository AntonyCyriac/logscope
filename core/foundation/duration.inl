/**
 * @file duration.inl
 * @brief Implements constexpr members of Duration.
 */

#pragma once

namespace scope::foundation
{

constexpr Duration::Duration() noexcept = default;

constexpr Duration::Duration(std::int64_t nanoseconds) noexcept
    : m_nanoseconds(nanoseconds)
{
}

constexpr Duration::Duration(int hours, int minutes, int seconds, int nanoseconds) noexcept
    : m_nanoseconds(((static_cast<std::int64_t>(hours) * 3600LL + static_cast<std::int64_t>(minutes) * 60LL +
                      static_cast<std::int64_t>(seconds)) *
                         1000000000LL) +
                    static_cast<std::int64_t>(nanoseconds))
{
}

constexpr int Duration::hours() const noexcept
{
    return static_cast<int>(m_nanoseconds / 3600000000000LL);
}

constexpr int Duration::minutes() const noexcept
{
    return static_cast<int>((m_nanoseconds / 60000000000LL) % 60);
}

constexpr int Duration::seconds() const noexcept
{
    return static_cast<int>((m_nanoseconds / 1000000000LL) % 60);
}

constexpr int Duration::nanoseconds() const noexcept
{
    return static_cast<int>(m_nanoseconds % 1000000000LL);
}

constexpr std::int64_t Duration::totalNanoseconds() const noexcept
{
    return m_nanoseconds;
}

constexpr bool operator==(const Duration& lhs, const Duration& rhs) noexcept
{
    return lhs.totalNanoseconds() == rhs.totalNanoseconds();
}

constexpr bool operator!=(const Duration& lhs, const Duration& rhs) noexcept
{
    return !(lhs == rhs);
}

constexpr bool operator<(const Duration& lhs, const Duration& rhs) noexcept
{
    return lhs.totalNanoseconds() < rhs.totalNanoseconds();
}

} // namespace scope::foundation
