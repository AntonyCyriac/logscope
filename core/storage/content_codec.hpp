/**
 * @file content_codec.hpp
 * @brief zlib compression helpers for persisted line content.
 */

#pragma once

#include <cstddef>
#include <string>
#include <string_view>

#include "foundation/result.hpp"

namespace scope::storage
{

[[nodiscard]] foundation::Result<std::string> compressZlib(std::string_view input);

[[nodiscard]] foundation::Result<std::string> decompressZlib(const void* data, std::size_t size);

} // namespace scope::storage
