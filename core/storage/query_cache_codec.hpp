/**
 * @file query_cache_codec.hpp
 * @brief Serialization for cached matching line numbers.
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace scope::storage
{

[[nodiscard]] std::string encodeLineNumbers(const std::vector<std::uint64_t>& lineNumbers) noexcept;

[[nodiscard]] std::vector<std::uint64_t> decodeLineNumbers(const std::string& blob) noexcept;

} // namespace scope::storage
