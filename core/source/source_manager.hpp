/**
 * @file source_manager.hpp
 * @brief Discovers, validates, and opens log sources.
 */

#pragma once

#include "discovery_census.hpp"
#include "foundation/path.hpp"
#include "foundation/result.hpp"
#include "source_dataset.hpp"
#include "source_open_options.hpp"

namespace scope::source
{

/**
 * @brief Manages the lifecycle of supported log sources.
 */
class SourceManager
{
  public:
    [[nodiscard]] foundation::Result<bool> validate(const foundation::Path& path,
                                                   const DiscoveryOptions& options = {}) const;

    [[nodiscard]] foundation::Result<SourceDataset> open(const foundation::Path& path) const;

    [[nodiscard]] foundation::Result<SourceDataset> open(const foundation::Path& path, OpenOptions options) const;
};

} // namespace scope::source
