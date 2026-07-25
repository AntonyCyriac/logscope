/**
 * @file fts_index.hpp
 * @brief FTS5 full-text index helpers for persisted line storage.
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "foundation/result.hpp"

struct sqlite3;
struct sqlite3_stmt;

namespace scope::storage
{

/**
 * @brief Escapes a user term for inclusion in an FTS5 MATCH expression.
 */
[[nodiscard]] std::string escapeFts5Literal(std::string_view term);

/**
 * @brief Builds a SQL predicate for contains(message|content, term) pushdown.
 */
[[nodiscard]] std::string buildFtsContainsSql(std::string_view column, std::string_view term);

/**
 * @brief Builds a SQL predicate for M7 text search across message and content.
 */
[[nodiscard]] std::string buildFtsSearchSql(std::string_view term);

[[nodiscard]] foundation::Result<bool> initializeFts5Schema(sqlite3* database);

[[nodiscard]] foundation::Result<bool> ensureFts5Index(sqlite3* database);

[[nodiscard]] foundation::Result<bool> insertFtsLine(sqlite3* database, sqlite3_stmt*& insertStatement,
                                                     std::int64_t lineId, std::string_view message,
                                                     std::string_view content);

void finalizeFtsInsertStatement(sqlite3_stmt*& insertStatement) noexcept;

} // namespace scope::storage
