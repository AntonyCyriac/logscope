/**
 * @file sqlite_connection.hpp
 * @brief Shared SQLite connection settings and error mapping.
 */

#pragma once

#include <sqlite3.h>

#include "foundation/error.hpp"
#include "foundation/result.hpp"

namespace scope::storage
{

/// Milliseconds to wait on SQLITE_BUSY before failing a write operation.
inline constexpr int kSqliteBusyTimeoutMs = 30000;

[[nodiscard]] bool isSqliteLockContention(int resultCode, int extendedResultCode) noexcept;

[[nodiscard]] foundation::Error makeSqliteError(sqlite3* database, int resultCode = 0);

[[nodiscard]] foundation::Result<bool> configureSqliteConnection(sqlite3* database);

} // namespace scope::storage
