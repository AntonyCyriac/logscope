/**
 * @file hash.hpp
 * @brief Hash utility functions.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace scope::foundation
{

/**
 * @brief Computes a 64-bit FNV-1a hash for a string.
 */
[[nodiscard]] std::uint64_t hashString(std::string_view value) noexcept;

/**
 * @brief Computes a 64-bit FNV-1a hash for a byte buffer.
 */
[[nodiscard]] std::uint64_t hashBytes(const void* data, std::size_t size) noexcept;

/**
 * @brief Combines a hash value into a running seed.
 */
void hashCombine(std::uint64_t& seed, std::uint64_t value) noexcept;

} // namespace scope::foundation
