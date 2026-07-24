/**
 * @file index_store_options.hpp
 * @brief Options applied when creating a persistent index store.
 */

#pragma once

#include <cstddef>

#include "storage_config.hpp"

namespace scope::storage
{

struct IndexStoreOptions
{
    bool compressContent{false};
    std::size_t compressThresholdBytes{256U};
    bool queryCacheEnabled{true};
    std::size_t queryCacheMaxEntries{64U};
};

[[nodiscard]] inline IndexStoreOptions indexStoreOptionsFromConfig(const StorageConfig& config) noexcept
{
    IndexStoreOptions options;
    options.compressContent = config.compressContent;
    options.compressThresholdBytes = config.compressThresholdBytes;
    options.queryCacheEnabled = config.queryCacheEnabled;
    options.queryCacheMaxEntries = config.queryCacheMaxEntries;

    return options;
}

} // namespace scope::storage
