/**
 * @file version.inl
 * @brief Implements constexpr members of Version.
 */

#pragma once

namespace scope::foundation
{

constexpr Version::Version() noexcept
    : m_major(0)
    , m_minor(0)
    , m_patch(0)
{
}

constexpr Version::Version(std::uint32_t major, std::uint32_t minor, std::uint32_t patch) noexcept
    : m_major(major)
    , m_minor(minor)
    , m_patch(patch)
{
}

constexpr std::uint32_t Version::major() const noexcept
{
    return m_major;
}

constexpr std::uint32_t Version::minor() const noexcept
{
    return m_minor;
}

constexpr std::uint32_t Version::patch() const noexcept
{
    return m_patch;
}

constexpr bool operator==(const Version& lhs, const Version& rhs) noexcept
{
    return lhs.m_major == rhs.m_major && lhs.m_minor == rhs.m_minor && lhs.m_patch == rhs.m_patch;
}

constexpr bool operator<(const Version& lhs, const Version& rhs) noexcept
{
    if (lhs.m_major != rhs.m_major)
    {
        return lhs.m_major < rhs.m_major;
    }

    if (lhs.m_minor != rhs.m_minor)
    {
        return lhs.m_minor < rhs.m_minor;
    }

    return lhs.m_patch < rhs.m_patch;
}

constexpr bool operator!=(const Version& lhs, const Version& rhs) noexcept
{
    return !(lhs == rhs);
}

constexpr bool operator<=(const Version& lhs, const Version& rhs) noexcept
{
    return !(rhs < lhs);
}

constexpr bool operator>(const Version& lhs, const Version& rhs) noexcept
{
    return rhs < lhs;
}

constexpr bool operator>=(const Version& lhs, const Version& rhs) noexcept
{
    return !(lhs < rhs);
}

} // namespace scope::foundation
