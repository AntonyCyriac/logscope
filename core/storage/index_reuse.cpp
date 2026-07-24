/**
 * @file index_reuse.cpp
 */

#include "index_reuse.hpp"

#include <filesystem>

#include <sqlite3.h>

#include "foundation/error.hpp"
#include "foundation/filesystem.hpp"
#include "index_store_factory.hpp"
#include "index_store_options.hpp"
#include "source_snapshot.hpp"
#include "sqlite_index_store.hpp"

namespace scope::storage
{

namespace
{

[[nodiscard]] foundation::Result<IndexStorePtr>
openReusableStore(const StorageConfig& config, const foundation::Path& databasePath,
                  const IndexFingerprint& fingerprint)
{
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

    const auto sqliteStore = std::static_pointer_cast<SqliteIndexStore>(*opened);

    if (sqliteStore->usesContentCompression() != config.compressContent)
    {
        return foundation::Result<IndexStorePtr>(foundation::Error(
            foundation::ErrorCode::InvalidArgument,
            "Index compression settings require rebuild from source log."));
    }

    sqliteStore->applyQueryCacheOptions(indexStoreOptionsFromConfig(config));

    return opened;
}

[[nodiscard]] foundation::Result<IndexStorePtr>
openIncrementalStore(const StorageConfig& config, const foundation::Path& databasePath,
                     const foundation::Path& sourcePath)
{
    const auto opened = SqliteIndexStore::open(databasePath);

    if (!opened)
    {
        return opened;
    }

    if ((*opened)->metadata().sourcePath.string() != sourcePath.string())
    {
        return foundation::Result<IndexStorePtr>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Persisted index source path mismatch."));
    }

    const auto sqliteStore = std::static_pointer_cast<SqliteIndexStore>(*opened);

    if (sqliteStore->usesContentCompression() != config.compressContent)
    {
        return foundation::Result<IndexStorePtr>(foundation::Error(
            foundation::ErrorCode::InvalidArgument,
            "Index compression settings require rebuild from source log."));
    }

    sqliteStore->applyQueryCacheOptions(indexStoreOptionsFromConfig(config));

    return opened;
}

[[nodiscard]] bool removeIndexDatabase(const foundation::Path& databasePath) noexcept
{
    std::error_code error;
    const std::filesystem::path basePath(databasePath.string());

    (void)std::filesystem::remove(basePath.string() + "-wal", error);
    error.clear();
    (void)std::filesystem::remove(basePath.string() + "-shm", error);
    error.clear();

    return std::filesystem::remove(basePath, error);
}

[[nodiscard]] bool removeTruncatedIndexIfNeeded(const foundation::Path& databasePath,
                                                const foundation::Path& sourcePath) noexcept
{
    sqlite3* database = nullptr;

    if (sqlite3_open_v2(databasePath.string().c_str(), &database, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK)
    {
        if (database != nullptr)
        {
            sqlite3_close(database);
        }

        return false;
    }

    const auto snapshot = readStoredSourceSnapshot(database);
    sqlite3_close(database);

    if (!snapshot)
    {
        return false;
    }

    const auto change = compareSourceChange(*snapshot, sourcePath);

    if (!change || *change != SourceChangeKind::Truncated)
    {
        return false;
    }

    return removeIndexDatabase(databasePath);
}

} // namespace

foundation::Result<IndexReusePrepareResult>
prepareIndexReuse(const StorageConfig& config, const IndexFingerprint& fingerprint,
                  const foundation::Path& sourcePath)
{
    IndexReusePrepareResult result;

    if (!config.reuseIndex)
    {
        return foundation::Result<IndexReusePrepareResult>(result);
    }

    const foundation::Path databasePath = resolveIndexPath(config, sourcePath, fingerprint);
    const auto exists = foundation::FileSystem::exists(databasePath);

    if (!exists || !*exists)
    {
        return foundation::Result<IndexReusePrepareResult>(result);
    }

    if (config.incrementalAppend && removeTruncatedIndexIfNeeded(databasePath, sourcePath))
    {
        return foundation::Result<IndexReusePrepareResult>(result);
    }

    if (!config.incrementalAppend)
    {
        const auto matches = IndexFingerprint::matchesSource(fingerprint, sourcePath);

        if (!matches || !*matches)
        {
            return foundation::Result<IndexReusePrepareResult>(result);
        }

        const auto opened = openReusableStore(config, databasePath, fingerprint);

        if (!opened)
        {
            return foundation::Result<IndexReusePrepareResult>(result);
        }

        result.mode = IndexReuseMode::Unchanged;
        result.store = *opened;

        return foundation::Result<IndexReusePrepareResult>(std::move(result));
    }

    IndexStorePtr store;
    {
        const auto opened = openIncrementalStore(config, databasePath, sourcePath);

        if (!opened)
        {
            return foundation::Result<IndexReusePrepareResult>(result);
        }

        store = *opened;
    }

    const auto sqliteStore = std::static_pointer_cast<SqliteIndexStore>(store);
    const auto snapshot = sqliteStore->storedSourceSnapshot();

    if (!snapshot)
    {
        return foundation::Result<IndexReusePrepareResult>(result);
    }

    const auto change = compareSourceChange(*snapshot, sourcePath);

    if (!change)
    {
        return foundation::Result<IndexReusePrepareResult>(change.error());
    }

    if (*change == SourceChangeKind::Truncated || *change == SourceChangeKind::Unknown)
    {
        store.reset();
        (void)removeIndexDatabase(databasePath);

        return foundation::Result<IndexReusePrepareResult>(result);
    }

    if (*change == SourceChangeKind::Unchanged)
    {
        result.mode = IndexReuseMode::Unchanged;
        result.store = std::move(store);

        return foundation::Result<IndexReusePrepareResult>(std::move(result));
    }

    sqliteStore->clearQueryCache();
    result.mode = IndexReuseMode::Append;
    result.store = std::move(store);
    result.linesToSkip = snapshot->totalLines;

    return foundation::Result<IndexReusePrepareResult>(std::move(result));
}

} // namespace scope::storage
