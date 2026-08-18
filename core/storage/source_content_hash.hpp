/**
 * @file source_content_hash.hpp
 * @brief SHA-256 helpers for source file integrity metadata.
 */

#pragma once

#include <cstdint>
#include <string>

#include "foundation/path.hpp"
#include "foundation/result.hpp"

namespace scope::storage
{

constexpr std::string_view kSourceContentSha256MetaKey = "source_content_sha256";
constexpr std::string_view kSourcePrefixSha256MetaKey = "source_prefix_sha256";

[[nodiscard]] foundation::Result<std::string> computeSourceSha256Hex(const foundation::Path& path);

[[nodiscard]] foundation::Result<std::string> computeSourcePrefixSha256Hex(const foundation::Path& path,
                                                                           std::uint64_t byteLength);

} // namespace scope::storage
