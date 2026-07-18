/**
 * @file timestamp.inl
 * @brief Implements constexpr members of Timestamp.
 */

#pragma once

namespace scope::foundation
{

constexpr Timestamp::Timestamp() noexcept = default;

constexpr Timestamp::Timestamp(std::int64_t unixNanoseconds) noexcept
    : m_unixNanoseconds(unixNanoseconds)
{
}

constexpr Timestamp Timestamp::fromUnixSeconds(std::int64_t unixSeconds) noexcept
{
    return Timestamp(unixSeconds * 1000000000LL);
}

constexpr std::int64_t Timestamp::unixNanoseconds() const noexcept
{
    return m_unixNanoseconds;
}

constexpr std::int64_t Timestamp::unixSeconds() const noexcept
{
    return m_unixNanoseconds / 1000000000LL;
}

constexpr bool operator==(const Timestamp& lhs, const Timestamp& rhs) noexcept
{
    return lhs.unixNanoseconds() == rhs.unixNanoseconds();
}

constexpr bool operator!=(const Timestamp& lhs, const Timestamp& rhs) noexcept
{
    return !(lhs == rhs);
}

constexpr bool operator<(const Timestamp& lhs, const Timestamp& rhs) noexcept
{
    return lhs.unixNanoseconds() < rhs.unixNanoseconds();
}

constexpr Timestamp operator+(const Timestamp& lhs, const Duration& rhs) noexcept
{
    return Timestamp(lhs.unixNanoseconds() + rhs.totalNanoseconds());
}

constexpr Timestamp operator-(const Timestamp& lhs, const Duration& rhs) noexcept
{
    return Timestamp(lhs.unixNanoseconds() - rhs.totalNanoseconds());
}

} // namespace scope::foundation
