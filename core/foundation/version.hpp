/**
 * @file version.hpp
 * @brief Defines the Version class used throughout the Scope Platform.
 */

#pragma once

#include <cstdint>
#include <string>

namespace scope::foundation
{

/**
 * @brief Represents a semantic version.
 *
 * A Version is an immutable value type consisting of
 * a major, minor, and patch component.
 */
class Version
{
  public:
    /**
     * @brief Constructs version 0.0.0.
     */
    constexpr Version() noexcept;

    /**
     * @brief Constructs a semantic version.
     *
     * @param major Major version.
     * @param minor Minor version.
     * @param patch Patch version.
     */
    constexpr Version(std::uint32_t major, std::uint32_t minor, std::uint32_t patch) noexcept;

    /**
     * @brief Returns the major version.
     */
    constexpr std::uint32_t major() const noexcept;

    /**
     * @brief Returns the minor version.
     */
    constexpr std::uint32_t minor() const noexcept;

    /**
     * @brief Returns the patch version.
     */
    constexpr std::uint32_t patch() const noexcept;

    /**
     * @brief Returns the version as a string.
     */
    std::string toString() const;

    friend constexpr bool operator==(const Version& lhs, const Version& rhs) noexcept;

    friend constexpr bool operator<(const Version& lhs, const Version& rhs) noexcept;

    friend constexpr bool operator!=(const Version& lhs, const Version& rhs) noexcept;

    friend constexpr bool operator<=(const Version& lhs, const Version& rhs) noexcept;

    friend constexpr bool operator>(const Version& lhs, const Version& rhs) noexcept;

    friend constexpr bool operator>=(const Version& lhs, const Version& rhs) noexcept;

  private:
    std::uint32_t m_major;

    std::uint32_t m_minor;

    std::uint32_t m_patch;
};

} // namespace scope::foundation

#include "version.inl"
