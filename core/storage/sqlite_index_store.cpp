/**
 * @file sqlite_index_store.cpp
 */

#include "sqlite_index_store.hpp"

#include <sqlite3.h>

#include <chrono>
#include <filesystem>
#include <sstream>
#include <string>

#include "content_codec.hpp"
#include "foundation/error.hpp"
#include "foundation/filesystem.hpp"
#include "foundation/string.hpp"
#include "index_fingerprint.hpp"
#include "source_snapshot.hpp"
#include "log_format.hpp"
#include "log_line_classifier.hpp"
#include "fts_index.hpp"
#include "query_cache_codec.hpp"
#include "query_cache_key.hpp"
#include "schema_version.hpp"
#include "sqlite_connection.hpp"

namespace scope::storage
{

namespace
{

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

[[nodiscard]] foundation::Result<bool> runSql(sqlite3* database, const char* sql)
{
    char* errorMessage = nullptr;
    const int result = sqlite3_exec(database, sql, nullptr, nullptr, &errorMessage);

    if (result != SQLITE_OK)
    {
        if (errorMessage != nullptr)
        {
            sqlite3_free(errorMessage);
        }

        return foundation::Result<bool>(makeSqliteError(database, result));
    }

    return foundation::Result<bool>(true);
}

[[nodiscard]] int levelToInt(const analysis::DetectedLogLevel level) noexcept
{
    switch (level)
    {
    case analysis::DetectedLogLevel::Blank:
        return 0;
    case analysis::DetectedLogLevel::Info:
        return 1;
    case analysis::DetectedLogLevel::Warn:
        return 2;
    case analysis::DetectedLogLevel::Error:
        return 3;
    case analysis::DetectedLogLevel::Other:
        return 4;
    }

    return 4;
}

[[nodiscard]] analysis::DetectedLogLevel levelFromInt(const int value) noexcept
{
    switch (value)
    {
    case 0:
        return analysis::DetectedLogLevel::Blank;
    case 1:
        return analysis::DetectedLogLevel::Info;
    case 2:
        return analysis::DetectedLogLevel::Warn;
    case 3:
        return analysis::DetectedLogLevel::Error;
    default:
        return analysis::DetectedLogLevel::Other;
    }
}

[[nodiscard]] std::string joinKeys(const std::vector<std::string>& keys)
{
    std::ostringstream output;

    for (std::size_t index = 0; index < keys.size(); ++index)
    {
        if (index > 0U)
        {
            output << ';';
        }

        output << keys[index];
    }

    return output.str();
}

[[nodiscard]] std::vector<std::string> splitKeys(const std::string& value)
{
    if (value.empty())
    {
        return {};
    }

    return foundation::split(value, ';');
}

[[nodiscard]] std::string logFormatToString(const analysis::LogFormat format)
{
    return std::string(analysis::logFormatName(format));
}

[[nodiscard]] analysis::LogFormat logFormatFromString(std::string_view value)
{
    if (value == "jsonl")
    {
        return analysis::LogFormat::JsonLines;
    }

    if (value == "plain")
    {
        return analysis::LogFormat::PlainText;
    }

    return analysis::LogFormat::PlainText;
}

[[nodiscard]] foundation::Result<bool> setMeta(sqlite3* database, const std::string& key,
                                               const std::string& value)
{
    sqlite3_stmt* statement = nullptr;
    const char* sql = "INSERT INTO meta(key, value) VALUES(?, ?) "
                      "ON CONFLICT(key) DO UPDATE SET value = excluded.value;";

    if (sqlite3_prepare_v2(database, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        return foundation::Result<bool>(
            makeSqliteError(database));
    }

    sqlite3_bind_text(statement, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, value.c_str(), -1, SQLITE_TRANSIENT);

    const int stepResult = sqlite3_step(statement);
    sqlite3_finalize(statement);

    if (stepResult != SQLITE_DONE)
    {
        return foundation::Result<bool>(
            makeSqliteError(database, stepResult));
    }

    return foundation::Result<bool>(true);
}

[[nodiscard]] foundation::Result<std::string> getMeta(sqlite3* database, const std::string& key)
{
    sqlite3_stmt* statement = nullptr;
    const char* sql = "SELECT value FROM meta WHERE key = ? LIMIT 1;";

    if (sqlite3_prepare_v2(database, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        return foundation::Result<std::string>(
            makeSqliteError(database));
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

[[nodiscard]] foundation::Result<bool> initializeSchemaV2(sqlite3* database)
{
    static constexpr const char* schemaSql = R"SQL(
CREATE TABLE IF NOT EXISTS meta (
  key TEXT PRIMARY KEY,
  value TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS lines (
  id INTEGER PRIMARY KEY,
  line_number INTEGER NOT NULL,
  level INTEGER NOT NULL,
  timestamp_unix INTEGER,
  message TEXT NOT NULL,
  content TEXT NOT NULL,
  correlation_id TEXT NOT NULL,
  top_level_keys_json TEXT NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_lines_level ON lines(level);
CREATE INDEX IF NOT EXISTS idx_lines_timestamp ON lines(timestamp_unix);
CREATE INDEX IF NOT EXISTS idx_lines_correlation ON lines(correlation_id);
CREATE TABLE IF NOT EXISTS line_json_fields (
  line_id INTEGER NOT NULL REFERENCES lines(id),
  field TEXT NOT NULL,
  value TEXT NOT NULL,
  PRIMARY KEY (line_id, field, value)
);
CREATE INDEX IF NOT EXISTS idx_ljf_field_value ON line_json_fields(field, value);
CREATE TABLE IF NOT EXISTS query_cache (
  cache_key TEXT PRIMARY KEY,
  line_ids_blob BLOB NOT NULL,
  created_at INTEGER NOT NULL,
  hit_count INTEGER NOT NULL DEFAULT 0
);
)SQL";

    const auto schemaResult = runSql(database, schemaSql);

    if (!schemaResult)
    {
        return schemaResult;
    }

    const auto versionResult =
        setMeta(database, "schema_version", std::to_string(kIndexSchemaVersionCurrent));

    if (!versionResult)
    {
        return versionResult;
    }

    return initializeFts5Schema(database);
}

[[nodiscard]] foundation::Result<int> readStoredSchemaVersion(sqlite3* database)
{
    const auto versionResult = getMeta(database, "schema_version");

    if (!versionResult)
    {
        return foundation::Result<int>(versionResult.error());
    }

    if (versionResult->empty())
    {
        return foundation::Result<int>(kIndexSchemaVersionV1);
    }

    try
    {
        return foundation::Result<int>(std::stoi(*versionResult));
    }
    catch (const std::exception&)
    {
        return foundation::Result<int>(foundation::Error(
            foundation::ErrorCode::ParseError, "Index database has invalid schema_version metadata."));
    }
}

[[nodiscard]] foundation::Result<bool> validateStoredSchemaVersion(sqlite3* database)
{
    const auto versionResult = readStoredSchemaVersion(database);

    if (!versionResult)
    {
        return foundation::Result<bool>(versionResult.error());
    }

    const int version = *versionResult;

    if (version < kIndexSchemaVersionCurrent)
    {
        return foundation::Result<bool>(foundation::Error(
            foundation::ErrorCode::InvalidArgument,
            "Index schema version " + std::to_string(version) + " requires rebuild from source log."));
    }

    if (version > kIndexSchemaVersionMaxSupported)
    {
        return foundation::Result<bool>(foundation::Error(
            foundation::ErrorCode::InvalidArgument,
            "Unsupported index schema version " + std::to_string(version) + " (maximum supported: " +
                std::to_string(kIndexSchemaVersionMaxSupported) + ")."));
    }

    return foundation::Result<bool>(true);
}

[[nodiscard]] foundation::Result<analysis::IndexedLine> decodeIndexedLine(sqlite3_stmt* statement)
{
    analysis::IndexedLine line{};
    line.lineNumber = static_cast<std::uint64_t>(sqlite3_column_int64(statement, 1));
    line.level = levelFromInt(sqlite3_column_int(statement, 2));

    if (sqlite3_column_type(statement, 3) != SQLITE_NULL)
    {
        line.timestamp = foundation::Timestamp::fromUnixSeconds(sqlite3_column_int64(statement, 3));
    }

    const unsigned char* message = sqlite3_column_text(statement, 4);
    const unsigned char* correlationId = sqlite3_column_text(statement, 6);
    const unsigned char* keys = sqlite3_column_text(statement, 7);

    if (message != nullptr)
    {
        line.messageExcerpt = reinterpret_cast<const char*>(message);
    }

    const int contentType = sqlite3_column_type(statement, 5);

    if (contentType == SQLITE_BLOB)
    {
        const void* blob = sqlite3_column_blob(statement, 5);
        const int blobSize = sqlite3_column_bytes(statement, 5);
        const auto decompressed = decompressZlib(blob, static_cast<std::size_t>(blobSize));

        if (!decompressed)
        {
            return foundation::Result<analysis::IndexedLine>(decompressed.error());
        }

        line.contentExcerpt = *decompressed;
    }
    else if (contentType == SQLITE_TEXT)
    {
        const unsigned char* content = sqlite3_column_text(statement, 5);

        if (content != nullptr)
        {
            line.contentExcerpt = reinterpret_cast<const char*>(content);
        }
    }

    if (correlationId != nullptr)
    {
        line.correlationId = reinterpret_cast<const char*>(correlationId);
    }

    if (keys != nullptr)
    {
        line.topLevelKeys = splitKeys(reinterpret_cast<const char*>(keys));
    }

    return foundation::Result<analysis::IndexedLine>(std::move(line));
}

[[nodiscard]] foundation::Result<bool> insertJsonFields(sqlite3* database, sqlite3_stmt*& insertStatement,
                                                          const sqlite3_int64 lineId,
                                                          const std::vector<std::pair<std::string, std::string>>&
                                                              fieldValues)
{
    if (fieldValues.empty())
    {
        return foundation::Result<bool>(true);
    }

    if (insertStatement == nullptr)
    {
        const char* sql = "INSERT INTO line_json_fields(line_id, field, value) VALUES(?, ?, ?);";

        if (sqlite3_prepare_v2(database, sql, -1, &insertStatement, nullptr) != SQLITE_OK)
        {
            return foundation::Result<bool>(
                makeSqliteError(database));
        }
    }

    for (const auto& fieldValue : fieldValues)
    {
        sqlite3_reset(insertStatement);
        sqlite3_clear_bindings(insertStatement);

        sqlite3_bind_int64(insertStatement, 1, lineId);
        sqlite3_bind_text(insertStatement, 2, fieldValue.first.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(insertStatement, 3, fieldValue.second.c_str(), -1, SQLITE_TRANSIENT);

        const int stepResult = sqlite3_step(insertStatement);

        if (stepResult != SQLITE_DONE)
        {
            return foundation::Result<bool>(
                makeSqliteError(database, stepResult));
        }
    }

    return foundation::Result<bool>(true);
}

[[nodiscard]] std::vector<std::pair<std::string, std::string>> loadJsonFieldValues(sqlite3* database,
                                                                                     const sqlite3_int64 lineId)
{
    std::vector<std::pair<std::string, std::string>> fieldValues;

    sqlite3_stmt* statement = nullptr;
    const char* sql = "SELECT field, value FROM line_json_fields WHERE line_id = ? ORDER BY field;";

    if (sqlite3_prepare_v2(database, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        return fieldValues;
    }

    sqlite3_bind_int64(statement, 1, lineId);

    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        const unsigned char* field = sqlite3_column_text(statement, 0);
        const unsigned char* value = sqlite3_column_text(statement, 1);

        if (field != nullptr && value != nullptr)
        {
            fieldValues.emplace_back(reinterpret_cast<const char*>(field),
                                     reinterpret_cast<const char*>(value));
        }
    }

    sqlite3_finalize(statement);

    return fieldValues;
}

[[nodiscard]] std::int64_t currentUnixTime() noexcept
{
    return std::chrono::duration_cast<std::chrono::seconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

[[nodiscard]] foundation::Result<bool> clearQueryCacheTable(sqlite3* database)
{
    return runSql(database, "DELETE FROM query_cache;");
}

[[nodiscard]] foundation::Result<bool> invalidateCacheOnSourceTruncate(sqlite3* database,
                                                                        const foundation::Path& sourcePath)
{
    const auto sourceSizeMeta = getMeta(database, "source_size");

    if (!sourceSizeMeta || sourceSizeMeta->empty())
    {
        return foundation::Result<bool>(true);
    }

    const auto currentSize = foundation::FileSystem::fileSize(sourcePath);

    if (!currentSize)
    {
        return foundation::Result<bool>(true);
    }

    try
    {
        if (*currentSize < std::stoull(*sourceSizeMeta))
        {
            return clearQueryCacheTable(database);
        }
    }
    catch (const std::exception&)
    {
        return foundation::Result<bool>(true);
    }

    return foundation::Result<bool>(true);
}

[[nodiscard]] std::optional<std::vector<std::uint64_t>> lookupQueryCacheLineNumbers(sqlite3* database,
                                                                                     const std::string& cacheKey)
{
    sqlite3_stmt* statement = nullptr;
    const char* sql = "SELECT line_ids_blob FROM query_cache WHERE cache_key = ? LIMIT 1;";

    if (sqlite3_prepare_v2(database, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        return std::nullopt;
    }

    sqlite3_bind_text(statement, 1, cacheKey.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<std::vector<std::uint64_t>> lineNumbers;

    if (sqlite3_step(statement) == SQLITE_ROW)
    {
        const void* blob = sqlite3_column_blob(statement, 0);
        const int blobSize = sqlite3_column_bytes(statement, 0);

        if (blob != nullptr && blobSize > 0)
        {
            const auto* bytes = static_cast<const char*>(blob);
            lineNumbers = decodeLineNumbers(std::string(bytes, static_cast<std::size_t>(blobSize)));
        }
    }

    sqlite3_finalize(statement);

    if (!lineNumbers.has_value())
    {
        return std::nullopt;
    }

    sqlite3_stmt* touchStatement = nullptr;
    const char* touchSql =
        "UPDATE query_cache SET hit_count = hit_count + 1, created_at = ? WHERE cache_key = ?;";

    if (sqlite3_prepare_v2(database, touchSql, -1, &touchStatement, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_int64(touchStatement, 1, currentUnixTime());
        sqlite3_bind_text(touchStatement, 2, cacheKey.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(touchStatement);
        sqlite3_finalize(touchStatement);
    }

    return lineNumbers;
}

[[nodiscard]] foundation::Result<bool> evictOldestQueryCacheEntries(sqlite3* database,
                                                                     const std::size_t maxEntries)
{
    if (maxEntries == 0U)
    {
        return clearQueryCacheTable(database);
    }

    sqlite3_stmt* countStatement = nullptr;

    if (sqlite3_prepare_v2(database, "SELECT COUNT(*) FROM query_cache;", -1, &countStatement, nullptr) != SQLITE_OK)
    {
        return foundation::Result<bool>(
            makeSqliteError(database));
    }

    std::size_t entryCount = 0U;

    if (sqlite3_step(countStatement) == SQLITE_ROW)
    {
        entryCount = static_cast<std::size_t>(sqlite3_column_int64(countStatement, 0));
    }

    sqlite3_finalize(countStatement);

    if (entryCount < maxEntries)
    {
        return foundation::Result<bool>(true);
    }

    const std::size_t deleteCount = entryCount - maxEntries + 1U;
    sqlite3_stmt* deleteStatement = nullptr;
    const char* deleteSql =
        "DELETE FROM query_cache WHERE cache_key IN (SELECT cache_key FROM query_cache ORDER BY created_at ASC "
        "LIMIT ?);";

    if (sqlite3_prepare_v2(database, deleteSql, -1, &deleteStatement, nullptr) != SQLITE_OK)
    {
        return foundation::Result<bool>(
            makeSqliteError(database));
    }

    sqlite3_bind_int64(deleteStatement, 1, static_cast<sqlite3_int64>(deleteCount));
    const int stepResult = sqlite3_step(deleteStatement);
    sqlite3_finalize(deleteStatement);

    if (stepResult != SQLITE_DONE)
    {
        return foundation::Result<bool>(
            makeSqliteError(database, stepResult));
    }

    return foundation::Result<bool>(true);
}

[[nodiscard]] foundation::Result<bool> storeQueryCacheEntry(sqlite3* database, const std::string& cacheKey,
                                                            const std::vector<std::uint64_t>& lineNumbers,
                                                            const std::size_t maxEntries)
{
    const auto evicted = evictOldestQueryCacheEntries(database, maxEntries);

    if (!evicted)
    {
        return evicted;
    }

    const std::string encoded = encodeLineNumbers(lineNumbers);

    sqlite3_stmt* statement = nullptr;
    const char* sql =
        "INSERT INTO query_cache(cache_key, line_ids_blob, created_at, hit_count) VALUES(?, ?, ?, 0) "
        "ON CONFLICT(cache_key) DO UPDATE SET line_ids_blob = excluded.line_ids_blob, created_at = excluded.created_at, "
        "hit_count = 0;";

    if (sqlite3_prepare_v2(database, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        return foundation::Result<bool>(
            makeSqliteError(database));
    }

    sqlite3_bind_text(statement, 1, cacheKey.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_blob(statement, 2, encoded.data(), static_cast<int>(encoded.size()), SQLITE_TRANSIENT);
    sqlite3_bind_int64(statement, 3, currentUnixTime());

    const int stepResult = sqlite3_step(statement);
    sqlite3_finalize(statement);

    if (stepResult != SQLITE_DONE)
    {
        return foundation::Result<bool>(
            makeSqliteError(database, stepResult));
    }

    return foundation::Result<bool>(true);
}

[[nodiscard]] foundation::Result<std::vector<analysis::IndexedLine>>
fetchLinesByLineNumbers(sqlite3* database, const std::vector<std::uint64_t>& lineNumbers)
{
    if (lineNumbers.empty())
    {
        return foundation::Result<std::vector<analysis::IndexedLine>>(std::vector<analysis::IndexedLine>{});
    }

    std::string sql =
        "SELECT id, line_number, level, timestamp_unix, message, content, correlation_id, top_level_keys_json "
        "FROM lines WHERE line_number IN (";

    for (std::size_t index = 0U; index < lineNumbers.size(); ++index)
    {
        if (index > 0U)
        {
            sql += ',';
        }

        sql += std::to_string(lineNumbers[index]);
    }

    sql += ") ORDER BY line_number;";

    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(database, sql.c_str(), -1, &statement, nullptr) != SQLITE_OK)
    {
        return foundation::Result<std::vector<analysis::IndexedLine>>(
            makeSqliteError(database));
    }

    std::vector<analysis::IndexedLine> lines;

    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        const sqlite3_int64 lineId = sqlite3_column_int64(statement, 0);
        const auto lineResult = decodeIndexedLine(statement);

        if (!lineResult)
        {
            sqlite3_finalize(statement);

            return foundation::Result<std::vector<analysis::IndexedLine>>(lineResult.error());
        }

        lines.push_back(std::move(*lineResult));
        lines.back().jsonFieldValues = loadJsonFieldValues(database, lineId);
    }

    sqlite3_finalize(statement);

    return foundation::Result<std::vector<analysis::IndexedLine>>(std::move(lines));
}

} // namespace

struct SqliteIndexStore::Impl
{
    static constexpr std::size_t writeBatchSize = 5000U;

    sqlite3* database{nullptr};
    foundation::Path databasePath;
    IndexMetadata metadata;
    std::uint64_t storedLines{0U};
    sqlite3_stmt* insertStatement{nullptr};
    sqlite3_stmt* jsonFieldInsertStatement{nullptr};
    sqlite3_stmt* ftsInsertStatement{nullptr};
    bool inWriteBatch{false};
    std::size_t linesInWriteBatch{0U};
    bool compressContent{false};
    std::size_t compressThresholdBytes{256U};
    bool indexUsesZlib{false};
    bool queryCacheEnabled{true};
    std::size_t queryCacheMaxEntries{64U};
};

namespace
{

[[nodiscard]] foundation::Result<bool> configureBulkInsertPragmas(sqlite3* database)
{
    const auto walResult = runSql(database, "PRAGMA journal_mode=WAL;");

    if (!walResult)
    {
        return walResult;
    }

    return runSql(database, "PRAGMA synchronous=NORMAL;");
}

} // namespace

foundation::Result<bool> SqliteIndexStore::beginWriteBatch()
{
    if (m_impl->inWriteBatch)
    {
        return foundation::Result<bool>(true);
    }

    const auto result = runSql(m_impl->database, "BEGIN IMMEDIATE;");

    if (!result)
    {
        return result;
    }

    m_impl->inWriteBatch = true;
    m_impl->linesInWriteBatch = 0U;

    return foundation::Result<bool>(true);
}

foundation::Result<bool> SqliteIndexStore::commitWriteBatch()
{
    if (!m_impl->inWriteBatch)
    {
        return foundation::Result<bool>(true);
    }

    const auto result = runSql(m_impl->database, "COMMIT;");

    if (!result)
    {
        return result;
    }

    m_impl->inWriteBatch = false;
    m_impl->linesInWriteBatch = 0U;

    return foundation::Result<bool>(true);
}

void SqliteIndexStore::rollbackWriteBatch() noexcept
{
    if (!m_impl->inWriteBatch)
    {
        return;
    }

    (void)runSql(m_impl->database, "ROLLBACK;");
    m_impl->inWriteBatch = false;
    m_impl->linesInWriteBatch = 0U;
}

foundation::Result<bool> SqliteIndexStore::bindAndInsertLine(const analysis::IndexedLine& line,
                                                             const std::string_view fullContent)
{
    if (m_impl->insertStatement == nullptr)
    {
        const char* sql =
            "INSERT INTO lines(line_number, level, timestamp_unix, message, content, correlation_id, "
            "top_level_keys_json) VALUES(?, ?, ?, ?, ?, ?, ?);";

        if (sqlite3_prepare_v2(m_impl->database, sql, -1, &m_impl->insertStatement, nullptr) != SQLITE_OK)
        {
            return foundation::Result<bool>(makeSqliteError(m_impl->database));
        }
    }

    sqlite3_stmt* statement = m_impl->insertStatement;

    sqlite3_reset(statement);
    sqlite3_clear_bindings(statement);

    sqlite3_bind_int64(statement, 1, static_cast<sqlite3_int64>(line.lineNumber));
    sqlite3_bind_int(statement, 2, levelToInt(line.level));

    if (line.timestamp.has_value())
    {
        sqlite3_bind_int64(statement, 3, static_cast<sqlite3_int64>(line.timestamp->unixSeconds()));
    }
    else
    {
        sqlite3_bind_null(statement, 3);
    }

    sqlite3_bind_text(statement, 4, line.messageExcerpt.c_str(), -1, SQLITE_TRANSIENT);

    if (m_impl->compressContent && fullContent.size() >= m_impl->compressThresholdBytes)
    {
        const auto compressed = compressZlib(fullContent);

        if (!compressed)
        {
            return foundation::Result<bool>(compressed.error());
        }

        if (compressed->size() < fullContent.size())
        {
            sqlite3_bind_blob(statement, 5, compressed->data(), static_cast<int>(compressed->size()),
                              SQLITE_TRANSIENT);
        }
        else
        {
            sqlite3_bind_text(statement, 5, fullContent.data(), static_cast<int>(fullContent.size()),
                              SQLITE_TRANSIENT);
        }
    }
    else
    {
        sqlite3_bind_text(statement, 5, fullContent.data(), static_cast<int>(fullContent.size()), SQLITE_TRANSIENT);
    }

    sqlite3_bind_text(statement, 6, line.correlationId.c_str(), -1, SQLITE_TRANSIENT);
    const std::string keys = joinKeys(line.topLevelKeys);
    sqlite3_bind_text(statement, 7, keys.c_str(), -1, SQLITE_TRANSIENT);

    const int stepResult = sqlite3_step(statement);

    if (stepResult != SQLITE_DONE)
    {
        return foundation::Result<bool>(makeSqliteError(m_impl->database, stepResult));
    }

    const sqlite3_int64 lineId = sqlite3_last_insert_rowid(m_impl->database);
    const auto ftsResult = insertFtsLine(m_impl->database, m_impl->ftsInsertStatement, lineId,
                                           line.messageExcerpt, fullContent);

    if (!ftsResult)
    {
        return ftsResult;
    }

    return insertJsonFields(m_impl->database, m_impl->jsonFieldInsertStatement,
                            sqlite3_last_insert_rowid(m_impl->database), line.jsonFieldValues);
}

SqliteIndexStore::SqliteIndexStore(std::unique_ptr<Impl> impl) noexcept
    : m_impl(std::move(impl))
{
}

SqliteIndexStore::~SqliteIndexStore()
{
    if (m_impl == nullptr)
    {
        return;
    }

    (void)commitWriteBatch();

    if (m_impl->insertStatement != nullptr)
    {
        sqlite3_finalize(m_impl->insertStatement);
        m_impl->insertStatement = nullptr;
    }

    if (m_impl->jsonFieldInsertStatement != nullptr)
    {
        sqlite3_finalize(m_impl->jsonFieldInsertStatement);
        m_impl->jsonFieldInsertStatement = nullptr;
    }

    finalizeFtsInsertStatement(m_impl->ftsInsertStatement);

    if (m_impl->database != nullptr)
    {
        sqlite3_close(m_impl->database);
        m_impl->database = nullptr;
    }
}

foundation::Result<IndexStorePtr> SqliteIndexStore::create(const foundation::Path& databasePath,
                                                           const IndexMetadata& metadata,
                                                           const IndexStoreOptions& options)
{
    std::error_code removeError;
    std::filesystem::remove(std::filesystem::path(databasePath.string()), removeError);

    if (sqlite3* database = nullptr;
        sqlite3_open(databasePath.string().c_str(), &database) != SQLITE_OK)
    {
        const foundation::Error error = database != nullptr
                                            ? makeSqliteError(database)
                                            : foundation::Error(foundation::ErrorCode::IOError,
                                                                "Unable to open database.");

        if (database != nullptr)
        {
            sqlite3_close(database);
        }

        return foundation::Result<IndexStorePtr>(error);
    }
    else
    {
        const auto connectionResult = configureSqliteConnection(database);

        if (!connectionResult)
        {
            sqlite3_close(database);

            return foundation::Result<IndexStorePtr>(connectionResult.error());
        }

        auto impl = std::make_unique<Impl>();
        impl->database = database;
        impl->databasePath = databasePath;
        impl->metadata = metadata;
        impl->compressContent = options.compressContent;
        impl->compressThresholdBytes = options.compressThresholdBytes;
        impl->indexUsesZlib = options.compressContent;
        impl->queryCacheEnabled = options.queryCacheEnabled;
        impl->queryCacheMaxEntries = options.queryCacheMaxEntries;

        const auto schemaResult = initializeSchemaV2(database);

        if (!schemaResult)
        {
            sqlite3_close(database);

            return foundation::Result<IndexStorePtr>(schemaResult.error());
        }

        const auto pragmaResult = configureBulkInsertPragmas(database);

        if (!pragmaResult)
        {
            sqlite3_close(database);

            return foundation::Result<IndexStorePtr>(pragmaResult.error());
        }

        const auto fingerprintResult = setMeta(database, "fingerprint", metadata.fingerprint);

        if (!fingerprintResult)
        {
            sqlite3_close(database);

            return foundation::Result<IndexStorePtr>(fingerprintResult.error());
        }

        const auto sourceResult = setMeta(database, "source_path", metadata.sourcePath.string());

        if (!sourceResult)
        {
            sqlite3_close(database);

            return foundation::Result<IndexStorePtr>(sourceResult.error());
        }

        const auto formatResult = setMeta(database, "format", logFormatToString(metadata.format));

        if (!formatResult)
        {
            sqlite3_close(database);

            return foundation::Result<IndexStorePtr>(formatResult.error());
        }

        const auto compressionResult =
            setMeta(database, "content_compression", options.compressContent ? "zlib" : "none");

        if (!compressionResult)
        {
            sqlite3_close(database);

            return foundation::Result<IndexStorePtr>(compressionResult.error());
        }

        return foundation::Result<IndexStorePtr>(
            IndexStorePtr(new SqliteIndexStore(std::move(impl))));
    }
}

foundation::Result<IndexStorePtr> SqliteIndexStore::open(const foundation::Path& databasePath)
{
    const auto existsResult = foundation::FileSystem::exists(databasePath);

    if (!existsResult || !*existsResult)
    {
        return foundation::Result<IndexStorePtr>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Index database does not exist."));
    }

    sqlite3* database = nullptr;

    if (sqlite3_open(databasePath.string().c_str(), &database) != SQLITE_OK)
    {
        const foundation::Error error = database != nullptr
                                            ? makeSqliteError(database)
                                            : foundation::Error(foundation::ErrorCode::IOError,
                                                                "Unable to open database.");

        if (database != nullptr)
        {
            sqlite3_close(database);
        }

        return foundation::Result<IndexStorePtr>(error);
    }

    const auto connectionResult = configureSqliteConnection(database);

    if (!connectionResult)
    {
        sqlite3_close(database);

        return foundation::Result<IndexStorePtr>(connectionResult.error());
    }

    auto impl = std::make_unique<Impl>();
    impl->database = database;
    impl->databasePath = databasePath;

    const auto schemaValidation = validateStoredSchemaVersion(database);

    if (!schemaValidation)
    {
        sqlite3_close(database);

        return foundation::Result<IndexStorePtr>(schemaValidation.error());
    }

    const auto fingerprintResult = getMeta(database, "fingerprint");

    if (!fingerprintResult || fingerprintResult->empty())
    {
        sqlite3_close(database);

        return foundation::Result<IndexStorePtr>(foundation::Error(
            foundation::ErrorCode::ParseError, "Index database is missing fingerprint metadata."));
    }

    const auto sourceResult = getMeta(database, "source_path");

    if (!sourceResult)
    {
        sqlite3_close(database);

        return foundation::Result<IndexStorePtr>(sourceResult.error());
    }

    const auto formatResult = getMeta(database, "format");
    const auto totalLinesResult = getMeta(database, "total_lines");
    const auto compressionResult = getMeta(database, "content_compression");

    IndexMetadata metadata;
    metadata.fingerprint = *fingerprintResult;
    metadata.sourcePath = foundation::Path(*sourceResult);
    metadata.format = formatResult.hasValue() ? logFormatFromString(*formatResult) : analysis::LogFormat::PlainText;

    if (totalLinesResult.hasValue() && !totalLinesResult->empty())
    {
        metadata.totalLines = std::stoull(*totalLinesResult);
    }

    impl->metadata = metadata;
    impl->indexUsesZlib = compressionResult.hasValue() && *compressionResult == "zlib";
    impl->compressContent = impl->indexUsesZlib;
    impl->queryCacheEnabled = true;
    impl->queryCacheMaxEntries = 64U;

    (void)invalidateCacheOnSourceTruncate(database, metadata.sourcePath);

    sqlite3_stmt* countStatement = nullptr;

    if (sqlite3_prepare_v2(database, "SELECT COUNT(*) FROM lines;", -1, &countStatement, nullptr) == SQLITE_OK &&
        sqlite3_step(countStatement) == SQLITE_ROW)
    {
        impl->storedLines = static_cast<std::uint64_t>(sqlite3_column_int64(countStatement, 0));
    }

    if (countStatement != nullptr)
    {
        sqlite3_finalize(countStatement);
    }

    const auto ftsResult = ensureFts5Index(database);

    if (!ftsResult)
    {
        sqlite3_close(database);

        return foundation::Result<IndexStorePtr>(ftsResult.error());
    }

    return foundation::Result<IndexStorePtr>(IndexStorePtr(new SqliteIndexStore(std::move(impl))));
}

foundation::Result<bool> SqliteIndexStore::appendLine(const analysis::IndexedLine& line,
                                                        std::string_view fullContent)
{
    const auto batchStart = beginWriteBatch();

    if (!batchStart)
    {
        return batchStart;
    }

    const auto insertResult = bindAndInsertLine(line, fullContent);

    if (!insertResult)
    {
        rollbackWriteBatch();

        return insertResult;
    }

    ++m_impl->storedLines;
    ++m_impl->linesInWriteBatch;

    if (m_impl->linesInWriteBatch >= Impl::writeBatchSize)
    {
        const auto commitResult = commitWriteBatch();

        if (!commitResult)
        {
            rollbackWriteBatch();

            return commitResult;
        }
    }

    return foundation::Result<bool>(true);
}

foundation::Result<bool> SqliteIndexStore::finalize(const std::uint64_t totalLines)
{
    const auto commitResult = commitWriteBatch();

    if (!commitResult)
    {
        rollbackWriteBatch();

        return commitResult;
    }

    m_impl->metadata.totalLines = totalLines;

    const auto totalLinesResult = setMeta(m_impl->database, "total_lines", std::to_string(totalLines));

    if (!totalLinesResult)
    {
        return totalLinesResult;
    }

    const auto indexedLineCountResult =
        setMeta(m_impl->database, "indexed_line_count", std::to_string(m_impl->storedLines));

    if (!indexedLineCountResult)
    {
        return indexedLineCountResult;
    }

    const auto sizeResult = foundation::FileSystem::fileSize(m_impl->metadata.sourcePath);

    if (!sizeResult)
    {
        return foundation::Result<bool>(sizeResult.error());
    }

    const auto sizeMetaResult =
        setMeta(m_impl->database, "source_size", std::to_string(*sizeResult));

    if (!sizeMetaResult)
    {
        return sizeMetaResult;
    }

    const auto mtimeResult = lastWriteTimeUnix(m_impl->metadata.sourcePath);

    if (!mtimeResult)
    {
        return foundation::Result<bool>(mtimeResult.error());
    }

    const auto mtimeMetaResult =
        setMeta(m_impl->database, "source_mtime", std::to_string(*mtimeResult));

    if (!mtimeMetaResult)
    {
        return mtimeMetaResult;
    }

    const auto fingerprintResult = IndexFingerprint::compute(m_impl->metadata.sourcePath);

    if (!fingerprintResult)
    {
        return foundation::Result<bool>(fingerprintResult.error());
    }

    m_impl->metadata.fingerprint = fingerprintResult->value();

    return setMeta(m_impl->database, "fingerprint", m_impl->metadata.fingerprint);
}

bool SqliteIndexStore::usesContentCompression() const noexcept
{
    return m_impl->indexUsesZlib;
}

std::uint64_t SqliteIndexStore::storedLineCount() const noexcept
{
    return m_impl->storedLines;
}

const foundation::Path& SqliteIndexStore::path() const noexcept
{
    return m_impl->databasePath;
}

const IndexMetadata& SqliteIndexStore::metadata() const noexcept
{
    return m_impl->metadata;
}

foundation::Result<std::vector<analysis::IndexedLine>> SqliteIndexStore::fetchLinesMatchingFts(
    const std::string& term) const
{
    return fetchLinesWhere(buildFtsSearchSql(term));
}

foundation::Result<std::vector<analysis::IndexedLine>> SqliteIndexStore::fetchAllLines() const
{
    return fetchLinesWhere("1 = 1");
}

foundation::Result<std::vector<analysis::IndexedLine>> SqliteIndexStore::fetchLinesWhere(
    const std::string& sqlWhereClause) const
{
    const std::string sql = "SELECT id, line_number, level, timestamp_unix, message, content, correlation_id, "
                            "top_level_keys_json FROM lines WHERE " +
                            sqlWhereClause + " ORDER BY line_number;";

    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(m_impl->database, sql.c_str(), -1, &statement, nullptr) != SQLITE_OK)
    {
        return foundation::Result<std::vector<analysis::IndexedLine>>(
            makeSqliteError(m_impl->database));
    }

    std::vector<analysis::IndexedLine> lines;

    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        const sqlite3_int64 lineId = sqlite3_column_int64(statement, 0);
        const auto lineResult = decodeIndexedLine(statement);

        if (!lineResult)
        {
            sqlite3_finalize(statement);

            return foundation::Result<std::vector<analysis::IndexedLine>>(lineResult.error());
        }

        lines.push_back(std::move(*lineResult));
        lines.back().jsonFieldValues = loadJsonFieldValues(m_impl->database, lineId);
    }

    sqlite3_finalize(statement);

    return foundation::Result<std::vector<analysis::IndexedLine>>(std::move(lines));
}

void SqliteIndexStore::applyQueryCacheOptions(const IndexStoreOptions& options) noexcept
{
    m_impl->queryCacheEnabled = options.queryCacheEnabled;
    m_impl->queryCacheMaxEntries = options.queryCacheMaxEntries;
}

bool SqliteIndexStore::queryCacheEnabled() const noexcept
{
    return m_impl->queryCacheEnabled;
}

void SqliteIndexStore::clearQueryCache() const noexcept
{
    (void)clearQueryCacheTable(m_impl->database);
}

foundation::Result<StoredSourceSnapshot> SqliteIndexStore::storedSourceSnapshot() const
{
    return readStoredSourceSnapshot(m_impl->database);
}

std::size_t SqliteIndexStore::queryCacheEntryCount() const noexcept
{
    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(m_impl->database, "SELECT COUNT(*) FROM query_cache;", -1, &statement, nullptr) != SQLITE_OK)
    {
        return 0U;
    }

    std::size_t entryCount = 0U;

    if (sqlite3_step(statement) == SQLITE_ROW)
    {
        entryCount = static_cast<std::size_t>(sqlite3_column_int64(statement, 0));
    }

    sqlite3_finalize(statement);

    return entryCount;
}

foundation::Result<QueryCacheFetchResult> SqliteIndexStore::fetchLinesMatchingPushdown(
    const std::string& canonicalFilter, const std::string& sqlWhere) const
{
    QueryCacheFetchResult result;

    if (!m_impl->queryCacheEnabled)
    {
        const auto fetched = fetchLinesWhere(sqlWhere);

        if (!fetched)
        {
            return foundation::Result<QueryCacheFetchResult>(fetched.error());
        }

        result.lines = *fetched;

        return foundation::Result<QueryCacheFetchResult>(std::move(result));
    }

    const std::string cacheKey =
        computeQueryCacheKey(m_impl->metadata.fingerprint, canonicalFilter, kIndexSchemaVersionCurrent);

    if (const auto cachedLineNumbers = lookupQueryCacheLineNumbers(m_impl->database, cacheKey))
    {
        const auto fetched = fetchLinesByLineNumbers(m_impl->database, *cachedLineNumbers);

        if (!fetched)
        {
            return foundation::Result<QueryCacheFetchResult>(fetched.error());
        }

        result.cacheHit = true;
        result.lines = *fetched;

        return foundation::Result<QueryCacheFetchResult>(std::move(result));
    }

    const auto fetched = fetchLinesWhere(sqlWhere);

    if (!fetched)
    {
        return foundation::Result<QueryCacheFetchResult>(fetched.error());
    }

    std::vector<std::uint64_t> lineNumbers;
    lineNumbers.reserve(fetched->size());

    for (const analysis::IndexedLine& line : *fetched)
    {
        lineNumbers.push_back(line.lineNumber);
    }

    (void)storeQueryCacheEntry(m_impl->database, cacheKey, lineNumbers, m_impl->queryCacheMaxEntries);

    result.lines = *fetched;

    return foundation::Result<QueryCacheFetchResult>(std::move(result));
}

} // namespace scope::storage
