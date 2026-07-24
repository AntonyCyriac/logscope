/**
 * @file sqlite_index_store.cpp
 */

#include "sqlite_index_store.hpp"

#include <sqlite3.h>

#include <sstream>
#include <string>

#include "foundation/error.hpp"
#include "foundation/filesystem.hpp"
#include "foundation/string.hpp"
#include "log_format.hpp"
#include "log_line_classifier.hpp"

namespace scope::storage
{

namespace
{

constexpr int schemaVersion = 1;

[[nodiscard]] foundation::Result<bool> runSql(sqlite3* database, const char* sql)
{
    char* errorMessage = nullptr;
    const int result = sqlite3_exec(database, sql, nullptr, nullptr, &errorMessage);

    if (result != SQLITE_OK)
    {
        const std::string message = errorMessage != nullptr ? errorMessage : sqlite3_errmsg(database);

        if (errorMessage != nullptr)
        {
            sqlite3_free(errorMessage);
        }

        return foundation::Result<bool>(foundation::Error(foundation::ErrorCode::IOError, message));
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
            foundation::Error(foundation::ErrorCode::IOError, sqlite3_errmsg(database)));
    }

    sqlite3_bind_text(statement, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, value.c_str(), -1, SQLITE_TRANSIENT);

    const int stepResult = sqlite3_step(statement);
    sqlite3_finalize(statement);

    if (stepResult != SQLITE_DONE)
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::IOError, sqlite3_errmsg(database)));
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
            foundation::Error(foundation::ErrorCode::IOError, sqlite3_errmsg(database)));
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

[[nodiscard]] foundation::Result<bool> initializeSchema(sqlite3* database)
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
)SQL";

    const auto schemaResult = runSql(database, schemaSql);

    if (!schemaResult)
    {
        return schemaResult;
    }

    return setMeta(database, "schema_version", std::to_string(schemaVersion));
}

[[nodiscard]] analysis::IndexedLine rowToIndexedLine(sqlite3_stmt* statement)
{
    analysis::IndexedLine line;
    line.lineNumber = static_cast<std::uint64_t>(sqlite3_column_int64(statement, 1));
    line.level = levelFromInt(sqlite3_column_int(statement, 2));

    if (sqlite3_column_type(statement, 3) != SQLITE_NULL)
    {
        line.timestamp = foundation::Timestamp::fromUnixSeconds(sqlite3_column_int64(statement, 3));
    }

    const unsigned char* message = sqlite3_column_text(statement, 4);
    const unsigned char* content = sqlite3_column_text(statement, 5);
    const unsigned char* correlationId = sqlite3_column_text(statement, 6);
    const unsigned char* keys = sqlite3_column_text(statement, 7);

    if (message != nullptr)
    {
        line.messageExcerpt = reinterpret_cast<const char*>(message);
    }

    if (content != nullptr)
    {
        line.contentExcerpt = reinterpret_cast<const char*>(content);
    }

    if (correlationId != nullptr)
    {
        line.correlationId = reinterpret_cast<const char*>(correlationId);
    }

    if (keys != nullptr)
    {
        line.topLevelKeys = splitKeys(reinterpret_cast<const char*>(keys));
    }

    return line;
}

} // namespace

struct SqliteIndexStore::Impl
{
    sqlite3* database{nullptr};
    foundation::Path databasePath;
    IndexMetadata metadata;
    std::uint64_t storedLines{0U};
};

SqliteIndexStore::SqliteIndexStore(std::unique_ptr<Impl> impl) noexcept
    : m_impl(std::move(impl))
{
}

SqliteIndexStore::~SqliteIndexStore()
{
    if (m_impl != nullptr && m_impl->database != nullptr)
    {
        sqlite3_close(m_impl->database);
        m_impl->database = nullptr;
    }
}

foundation::Result<IndexStorePtr> SqliteIndexStore::create(const foundation::Path& databasePath,
                                                           const IndexMetadata& metadata)
{
    if (sqlite3* database = nullptr;
        sqlite3_open(databasePath.string().c_str(), &database) != SQLITE_OK)
    {
        const std::string message = database != nullptr ? sqlite3_errmsg(database) : "Unable to open database.";

        if (database != nullptr)
        {
            sqlite3_close(database);
        }

        return foundation::Result<IndexStorePtr>(foundation::Error(foundation::ErrorCode::IOError, message));
    }
    else
    {
        auto impl = std::make_unique<Impl>();
        impl->database = database;
        impl->databasePath = databasePath;
        impl->metadata = metadata;

        const auto schemaResult = initializeSchema(database);

        if (!schemaResult)
        {
            sqlite3_close(database);

            return foundation::Result<IndexStorePtr>(schemaResult.error());
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
        const std::string message = database != nullptr ? sqlite3_errmsg(database) : "Unable to open database.";

        if (database != nullptr)
        {
            sqlite3_close(database);
        }

        return foundation::Result<IndexStorePtr>(foundation::Error(foundation::ErrorCode::IOError, message));
    }

    auto impl = std::make_unique<Impl>();
    impl->database = database;
    impl->databasePath = databasePath;

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

    IndexMetadata metadata;
    metadata.fingerprint = *fingerprintResult;
    metadata.sourcePath = foundation::Path(*sourceResult);
    metadata.format = formatResult.hasValue() ? logFormatFromString(*formatResult) : analysis::LogFormat::PlainText;

    if (totalLinesResult.hasValue() && !totalLinesResult->empty())
    {
        metadata.totalLines = std::stoull(*totalLinesResult);
    }

    impl->metadata = metadata;

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

    return foundation::Result<IndexStorePtr>(IndexStorePtr(new SqliteIndexStore(std::move(impl))));
}

foundation::Result<bool> SqliteIndexStore::appendLine(const analysis::IndexedLine& line,
                                                        std::string_view fullContent)
{
    sqlite3_stmt* statement = nullptr;
    const char* sql =
        "INSERT INTO lines(line_number, level, timestamp_unix, message, content, correlation_id, "
        "top_level_keys_json) VALUES(?, ?, ?, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(m_impl->database, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::IOError, sqlite3_errmsg(m_impl->database)));
    }

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
    sqlite3_bind_text(statement, 5, fullContent.data(), static_cast<int>(fullContent.size()), SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 6, line.correlationId.c_str(), -1, SQLITE_TRANSIENT);
    const std::string keys = joinKeys(line.topLevelKeys);
    sqlite3_bind_text(statement, 7, keys.c_str(), -1, SQLITE_TRANSIENT);

    const int stepResult = sqlite3_step(statement);
    sqlite3_finalize(statement);

    if (stepResult != SQLITE_DONE)
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::IOError, sqlite3_errmsg(m_impl->database)));
    }

    ++m_impl->storedLines;

    return foundation::Result<bool>(true);
}

foundation::Result<bool> SqliteIndexStore::finalize(const std::uint64_t totalLines)
{
    m_impl->metadata.totalLines = totalLines;

    return setMeta(m_impl->database, "total_lines", std::to_string(totalLines));
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
        return foundation::Result<std::vector<analysis::IndexedLine>>(foundation::Error(
            foundation::ErrorCode::IOError, sqlite3_errmsg(m_impl->database)));
    }

    std::vector<analysis::IndexedLine> lines;

    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        lines.push_back(rowToIndexedLine(statement));
    }

    sqlite3_finalize(statement);

    return foundation::Result<std::vector<analysis::IndexedLine>>(std::move(lines));
}

} // namespace scope::storage
