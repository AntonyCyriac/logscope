/**
 * @file source_snapshot.cpp
 */

#include "source_snapshot.hpp"

#include <chrono>
#include <filesystem>

#include "foundation/error.hpp"
#include "foundation/filesystem.hpp"
#include "source_content_hash.hpp"
#include "sqlite_connection.hpp"
#include "sqlite3.h"

namespace scope::storage
{

namespace
{

[[nodiscard]] foundation::Result<std::string> getMeta(sqlite3* database, const std::string& key)
{
    sqlite3_stmt* statement = nullptr;
    const char* sql = "SELECT value FROM meta WHERE key = ? LIMIT 1;";

    if (sqlite3_prepare_v2(database, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        return foundation::Result<std::string>(makeSqliteError(database));
    }

    sqlite3_bind_text(statement, 1, key.c_str(), -1, SQLITE_TRANSIENT);

    std::string value;

    if (sqlite3_step(statement) == SQLITE_ROW)
    {
        const unsigned char* text = sqlite3_column_text(statement, 0);

        if (text != nullptr)
        {
            value = reinterpret_cast<const char*>(text);
        }
    }

    sqlite3_finalize(statement);

    return foundation::Result<std::string>(std::move(value));
}

[[nodiscard]] foundation::Result<std::uint64_t> readMetaU64(sqlite3* database, const std::string& key)
{
    const auto value = getMeta(database, key);

    if (!value)
    {
        return foundation::Result<std::uint64_t>(value.error());
    }

    if (value->empty())
    {
        return foundation::Result<std::uint64_t>(0U);
    }

    try
    {
        return foundation::Result<std::uint64_t>(std::stoull(*value));
    }
    catch (const std::exception&)
    {
        return foundation::Result<std::uint64_t>(foundation::Error(
            foundation::ErrorCode::ParseError, "Invalid numeric meta value for " + key));
    }
}

[[nodiscard]] foundation::Result<std::int64_t> lastWriteTimeUnix(const foundation::Path& path)
{
    std::error_code error;
    const auto writeTime =
        std::filesystem::last_write_time(std::filesystem::path(path.string()), error);

    if (error)
    {
        return foundation::Result<std::int64_t>(
            foundation::Error(foundation::ErrorCode::IOError, error.message()));
    }

    const auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        writeTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());

    return foundation::Result<std::int64_t>(
        std::chrono::duration_cast<std::chrono::seconds>(sctp.time_since_epoch()).count());
}

} // namespace

foundation::Result<StoredSourceSnapshot> readStoredSourceSnapshot(sqlite3* database)
{
    StoredSourceSnapshot snapshot;

    const auto sourcePath = getMeta(database, "source_path");

    if (!sourcePath)
    {
        return foundation::Result<StoredSourceSnapshot>(sourcePath.error());
    }

    snapshot.sourcePath = *sourcePath;

    const auto sourceSize = readMetaU64(database, "source_size");

    if (!sourceSize)
    {
        return foundation::Result<StoredSourceSnapshot>(sourceSize.error());
    }

    snapshot.sourceSize = *sourceSize;

    const auto totalLines = readMetaU64(database, "total_lines");

    if (!totalLines)
    {
        return foundation::Result<StoredSourceSnapshot>(totalLines.error());
    }

    snapshot.totalLines = *totalLines;

    const auto indexedLineCount = readMetaU64(database, "indexed_line_count");

    if (!indexedLineCount)
    {
        return foundation::Result<StoredSourceSnapshot>(indexedLineCount.error());
    }

    snapshot.indexedLineCount = *indexedLineCount;

    const auto sourceMtime = getMeta(database, "source_mtime");

    if (!sourceMtime)
    {
        return foundation::Result<StoredSourceSnapshot>(sourceMtime.error());
    }

    if (!sourceMtime->empty())
    {
        try
        {
            snapshot.sourceMtime = std::stoll(*sourceMtime);
        }
        catch (const std::exception&)
        {
            return foundation::Result<StoredSourceSnapshot>(foundation::Error(
                foundation::ErrorCode::ParseError, "Invalid source_mtime metadata."));
        }
    }

    const auto sourceContentSha256 = getMeta(database, std::string(kSourceContentSha256MetaKey));

    if (!sourceContentSha256)
    {
        return foundation::Result<StoredSourceSnapshot>(sourceContentSha256.error());
    }

    snapshot.sourceContentSha256 = *sourceContentSha256;

    const auto sourcePrefixSha256 = getMeta(database, std::string(kSourcePrefixSha256MetaKey));

    if (!sourcePrefixSha256)
    {
        return foundation::Result<StoredSourceSnapshot>(sourcePrefixSha256.error());
    }

    snapshot.sourcePrefixSha256 = *sourcePrefixSha256;

    return foundation::Result<StoredSourceSnapshot>(std::move(snapshot));
}

foundation::Result<SourceChangeKind> compareSourceChange(const StoredSourceSnapshot& stored,
                                                         const foundation::Path& sourcePath)
{
    if (stored.sourcePath != sourcePath.string())
    {
        return foundation::Result<SourceChangeKind>(SourceChangeKind::Unknown);
    }

    if (stored.sourceSize == 0U && stored.totalLines == 0U)
    {
        return foundation::Result<SourceChangeKind>(SourceChangeKind::Unknown);
    }

    const auto currentSize = foundation::FileSystem::fileSize(sourcePath);

    if (!currentSize)
    {
        return foundation::Result<SourceChangeKind>(currentSize.error());
    }

    if (*currentSize < stored.sourceSize)
    {
        return foundation::Result<SourceChangeKind>(SourceChangeKind::Truncated);
    }

    if (stored.sourceContentSha256.empty() || stored.sourcePrefixSha256.empty())
    {
        return foundation::Result<SourceChangeKind>(SourceChangeKind::Unknown);
    }

    if (*currentSize == stored.sourceSize)
    {
        const auto currentHash = computeSourceSha256Hex(sourcePath);

        if (!currentHash)
        {
            return foundation::Result<SourceChangeKind>(currentHash.error());
        }

        if (*currentHash != stored.sourceContentSha256)
        {
            return foundation::Result<SourceChangeKind>(SourceChangeKind::Unknown);
        }

        return foundation::Result<SourceChangeKind>(SourceChangeKind::Unchanged);
    }

    const auto prefixHash = computeSourcePrefixSha256Hex(sourcePath, stored.sourceSize);

    if (!prefixHash)
    {
        return foundation::Result<SourceChangeKind>(prefixHash.error());
    }

    if (*prefixHash != stored.sourcePrefixSha256)
    {
        return foundation::Result<SourceChangeKind>(SourceChangeKind::Unknown);
    }

    return foundation::Result<SourceChangeKind>(SourceChangeKind::Grown);
}

} // namespace scope::storage
