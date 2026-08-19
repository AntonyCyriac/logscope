/**
 * @file fts_index.cpp
 */

#include "fts_index.hpp"

#include <optional>
#include <sqlite3.h>

#include "content_codec.hpp"
#include "foundation/error.hpp"
#include "sqlite_connection.hpp"

namespace scope::storage
{

namespace
{

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

[[nodiscard]] bool ftsTableExists(sqlite3* database) noexcept
{
    sqlite3_stmt* statement = nullptr;
    const char* sql =
        "SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = 'lines_fts' LIMIT 1;";

    if (sqlite3_prepare_v2(database, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        return false;
    }

    const bool exists = sqlite3_step(statement) == SQLITE_ROW;
    sqlite3_finalize(statement);

    return exists;
}

[[nodiscard]] std::optional<std::size_t> countRows(sqlite3* database, const char* sql)
{
    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(database, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        return std::nullopt;
    }

    std::optional<std::size_t> count;

    if (sqlite3_step(statement) == SQLITE_ROW)
    {
        count = static_cast<std::size_t>(sqlite3_column_int64(statement, 0));
    }

    sqlite3_finalize(statement);

    return count;
}

[[nodiscard]] foundation::Result<bool> backfillFts5Index(sqlite3* database)
{
    sqlite3_stmt* statement = nullptr;
    const char* sql =
        "SELECT id, message, content FROM lines ORDER BY id;";

    if (sqlite3_prepare_v2(database, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        return foundation::Result<bool>(
            makeSqliteError(database));
    }

    sqlite3_stmt* insertStatement = nullptr;

    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        const sqlite3_int64 lineId = sqlite3_column_int64(statement, 0);
        const unsigned char* message = sqlite3_column_text(statement, 1);
        const std::string messageText =
            message != nullptr ? reinterpret_cast<const char*>(message) : std::string{};

        std::string contentText;

        if (sqlite3_column_type(statement, 2) == SQLITE_BLOB)
        {
            const void* blob = sqlite3_column_blob(statement, 2);
            const int blobSize = sqlite3_column_bytes(statement, 2);
            const auto decompressed = decompressZlib(blob, static_cast<std::size_t>(blobSize));

            if (!decompressed)
            {
                sqlite3_finalize(statement);
                finalizeFtsInsertStatement(insertStatement);

                return foundation::Result<bool>(decompressed.error());
            }

            contentText = *decompressed;
        }
        else
        {
            const unsigned char* content = sqlite3_column_text(statement, 2);

            if (content != nullptr)
            {
                contentText = reinterpret_cast<const char*>(content);
            }
        }

        const auto inserted =
            insertFtsLine(database, insertStatement, lineId, messageText, contentText);

        if (!inserted)
        {
            sqlite3_finalize(statement);
            finalizeFtsInsertStatement(insertStatement);

            return inserted;
        }
    }

    sqlite3_finalize(statement);
    finalizeFtsInsertStatement(insertStatement);

    return foundation::Result<bool>(true);
}

} // namespace

bool ftsContainsIsFaithful(const std::string_view term) noexcept
{
    for (const unsigned char character : term)
    {
        if (character > 127U)
        {
            return false;
        }
    }

    return true;
}

std::string escapeFts5Literal(const std::string_view term)
{
    std::string escaped;
    escaped.reserve(term.size() + 2U);
    escaped.push_back('"');

    for (const char character : term)
    {
        if (character == '"')
        {
            escaped.append("\"\"");
        }
        else
        {
            escaped.push_back(character);
        }
    }

    escaped.push_back('"');

    return escaped;
}

std::string buildFtsContainsSql(const std::string_view column, const std::string_view term)
{
    const std::string matchExpression = std::string(column) + " : " + escapeFts5Literal(term);

    return "lines.id IN (SELECT rowid FROM lines_fts WHERE lines_fts MATCH '" + matchExpression + "')";
}

std::string buildFtsSearchSql(const std::string_view term)
{
    const std::string matchExpression = escapeFts5Literal(term);

    return "lines.id IN (SELECT rowid FROM lines_fts WHERE lines_fts MATCH '" + matchExpression + "')";
}

foundation::Result<bool> initializeFts5Schema(sqlite3* database)
{
    static constexpr const char* schemaSql = R"SQL(
CREATE VIRTUAL TABLE IF NOT EXISTS lines_fts USING fts5(
  message,
  content,
  tokenize='unicode61'
);
)SQL";

    return runSql(database, schemaSql);
}

foundation::Result<bool> ensureFts5Index(sqlite3* database)
{
    if (!ftsTableExists(database))
    {
        const auto created = initializeFts5Schema(database);

        if (!created)
        {
            return created;
        }
    }

    const auto lineCount = countRows(database, "SELECT COUNT(*) FROM lines;");
    const auto ftsCount = countRows(database, "SELECT COUNT(*) FROM lines_fts;");

    if (!lineCount.has_value() || !ftsCount.has_value())
    {
        return foundation::Result<bool>(foundation::Error(
            foundation::ErrorCode::IOError, "Unable to inspect FTS index population state."));
    }

    if (*lineCount > 0U && *ftsCount == 0U)
    {
        return backfillFts5Index(database);
    }

    return foundation::Result<bool>(true);
}

foundation::Result<bool> insertFtsLine(sqlite3* database, sqlite3_stmt*& insertStatement,
                                       const std::int64_t lineId, const std::string_view message,
                                       const std::string_view content)
{
    if (insertStatement == nullptr)
    {
        const char* sql = "INSERT INTO lines_fts(rowid, message, content) VALUES(?, ?, ?);";

        if (sqlite3_prepare_v2(database, sql, -1, &insertStatement, nullptr) != SQLITE_OK)
        {
            return foundation::Result<bool>(
                makeSqliteError(database));
        }
    }

    sqlite3_reset(insertStatement);
    sqlite3_clear_bindings(insertStatement);

    sqlite3_bind_int64(insertStatement, 1, lineId);
    sqlite3_bind_text(insertStatement, 2, message.data(), static_cast<int>(message.size()), SQLITE_TRANSIENT);
    sqlite3_bind_text(insertStatement, 3, content.data(), static_cast<int>(content.size()), SQLITE_TRANSIENT);

    const int stepResult = sqlite3_step(insertStatement);

    if (stepResult != SQLITE_DONE)
    {
        return foundation::Result<bool>(
            makeSqliteError(database));
    }

    return foundation::Result<bool>(true);
}

void finalizeFtsInsertStatement(sqlite3_stmt*& insertStatement) noexcept
{
    if (insertStatement != nullptr)
    {
        sqlite3_finalize(insertStatement);
        insertStatement = nullptr;
    }
}

} // namespace scope::storage
