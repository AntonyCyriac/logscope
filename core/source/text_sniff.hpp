/**
 * @file text_sniff.hpp
 * @brief Lightweight text/binary classification for discovery.
 */

#pragma once

#include "discovery_census.hpp"
#include "foundation/path.hpp"
#include "foundation/result.hpp"

namespace scope::source
{

[[nodiscard]] foundation::Result<HeadSniffResult> sniffFileHead(const foundation::Path& path);

} // namespace scope::source
