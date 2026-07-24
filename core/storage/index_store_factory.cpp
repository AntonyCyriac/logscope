/**
 * @file index_store_factory.cpp
 */

#include "index_store_factory.hpp"

#include <filesystem>

#include "foundation/error.hpp"
#include "foundation/filesystem.hpp"
#include "index_reuse.hpp"
#include "index_store_options.hpp"
#include "sqlite_index_store.hpp"

namespace scope::storage
{

foundation::Path resolveIndexPath(const StorageConfig& config, const foundation::Path& sourcePath,
                                  const IndexFingerprint& fingerprint)
{
    if (config.indexPath.has_value())
    {
        return *config.indexPath;
    }

    if (config.incrementalAppend)
    {
        const auto stableKey = IndexFingerprint::stablePathKey(sourcePath);

        if (stableKey)
        {
            return config.indexDirectory.append(stableKey->value() + ".db");
        }
    }

    return config.indexDirectory.append(fingerprint.value() + ".db");
}

foundation::Result<IndexStorePtr> createIndexStore(const StorageConfig& config,
                                                   const IndexFingerprint& fingerprint,
                                                   const foundation::Path& sourcePath,
                                                   const analysis::LogFormat format)
{
    const foundation::Path databasePath = resolveIndexPath(config, sourcePath, fingerprint);

    if (!config.indexPath.has_value())
    {
        std::error_code error;
        std::filesystem::create_directories(std::filesystem::path(config.indexDirectory.string()), error);

        if (error)
        {
            return foundation::Result<IndexStorePtr>(
                foundation::Error(foundation::ErrorCode::IOError, error.message()));
        }
    }

    IndexMetadata metadata;
    metadata.fingerprint = fingerprint.value();
    metadata.sourcePath = sourcePath;
    metadata.format = format;

    return SqliteIndexStore::create(databasePath, metadata, indexStoreOptionsFromConfig(config));
}

foundation::Result<IndexStorePtr> tryOpenReusableIndex(const StorageConfig& config,
                                                       const IndexFingerprint& fingerprint,
                                                       const foundation::Path& sourcePath)
{
    const auto prepared = prepareIndexReuse(config, fingerprint, sourcePath);

    if (!prepared)
    {
        return foundation::Result<IndexStorePtr>(prepared.error());
    }

    if (prepared->mode != IndexReuseMode::Unchanged || prepared->store == nullptr)
    {
        return foundation::Result<IndexStorePtr>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Source fingerprint does not match persisted index."));
    }

    return foundation::Result<IndexStorePtr>(prepared->store);
}

} // namespace scope::storage
