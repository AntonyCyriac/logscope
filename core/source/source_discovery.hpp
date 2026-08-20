/**
 * @file source_discovery.hpp
 * @brief Recursive discovery with census accounting.
 */

#pragma once

#include "discovery_census.hpp"
#include "foundation/path.hpp"
#include "foundation/result.hpp"
#include "rotation_grouper.hpp"

namespace scope::source
{

struct DiscoveryScanResult
{
    DiscoveryCensus census;
    std::vector<IngestStream> ingestStreams;
};

[[nodiscard]] foundation::Result<DiscoveryScanResult> discoverSource(const foundation::Path& root,
                                                                     const DiscoveryOptions& options);

} // namespace scope::source
