/**
 * @file index_store_factory.cpp
 */

#include "index_store_factory.hpp"

#include <filesystem>

#include "foundation/error.hpp"
#include "foundation/filesystem.hpp"
#include "sqlite_index_store.hpp"

namespace scope::storage
{

foundation::Path resolveIndexPath(const StorageConfig& config, const IndexFingerprint& fingerprint)
{
    if (config.indexPath.has_value())
    {
        return *config.indexPath;
    }

    return config.indexDirectory.append(fingerprint.value() + ".db");
}

foundation::Result<IndexStorePtr> createIndexStore(const StorageConfig& config,
                                                   const IndexFingerprint& fingerprint,
                                                   const foundation::Path& sourcePath,
                                                   const analysis::LogFormat format)
{
    const foundation::Path databasePath = resolveIndexPath(config, fingerprint);

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

    return SqliteIndexStore::create(databasePath, metadata);
}

foundation::Result<IndexStorePtr> tryOpenReusableIndex(const StorageConfig& config,
                                                       const IndexFingerprint& fingerprint,
                                                       const foundation::Path& sourcePath)
{
    if (!config.reuseIndex)
    {
        return foundation::Result<IndexStorePtr>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Index reuse is disabled."));
    }

    const auto matches = IndexFingerprint::matchesSource(fingerprint, sourcePath);

    if (!matches || !*matches)
    {
        return foundation::Result<IndexStorePtr>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Source fingerprint does not match persisted index."));
    }

    const foundation::Path databasePath = resolveIndexPath(config, fingerprint);
    const auto opened = SqliteIndexStore::open(databasePath);

    if (!opened)
    {
        return opened;
    }

    if ((*opened)->metadata().fingerprint != fingerprint.value())
    {
        return foundation::Result<IndexStorePtr>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Persisted index fingerprint mismatch."));
    }

    return opened;
}

} // namespace scope::storage
