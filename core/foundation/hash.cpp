/**
 * @file hash.cpp
 * @brief Hash utility implementation.
 */

#include "hash.hpp"

#include <cstdint>
#include <cstring>

namespace scope::foundation
{

namespace
{

constexpr std::uint64_t FNV_OFFSET_BASIS = 14695981039346656037ULL;

constexpr std::uint64_t FNV_PRIME = 1099511628211ULL;

std::uint64_t fnv1a(std::uint64_t hash, const unsigned char* bytes, std::size_t size)
{
    for (std::size_t index = 0; index < size; ++index)
    {
        hash ^= static_cast<std::uint64_t>(bytes[index]);
        hash *= FNV_PRIME;
    }

    return hash;
}

} // namespace

std::uint64_t hashString(std::string_view value) noexcept
{
    return fnv1a(FNV_OFFSET_BASIS, reinterpret_cast<const unsigned char*>(value.data()), value.size());
}

std::uint64_t hashBytes(const void* data, std::size_t size) noexcept
{
    if (data == nullptr || size == 0)
    {
        return FNV_OFFSET_BASIS;
    }

    return fnv1a(FNV_OFFSET_BASIS, static_cast<const unsigned char*>(data), size);
}

void hashCombine(std::uint64_t& seed, std::uint64_t value) noexcept
{
    seed ^= value + 0x9E3779B97F4A7C15ULL + (seed << 6U) + (seed >> 2U);
}

} // namespace scope::foundation
