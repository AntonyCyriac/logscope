/**
 * @file sqlite_connection.cpp
 */

#include "sqlite_connection.hpp"

namespace scope::storage
{

namespace
{

constexpr const char* kIndexLockedMessage = "Index is locked by another process.";

} // namespace

bool isSqliteLockContention(const int resultCode, const int extendedResultCode) noexcept
{
    if (resultCode == SQLITE_BUSY || resultCode == SQLITE_LOCKED)
    {
        return true;
    }

    if (resultCode == SQLITE_IOERR)
    {
        switch (extendedResultCode)
        {
        case SQLITE_IOERR_BLOCKED:
        case SQLITE_IOERR_LOCK:
        case SQLITE_IOERR_RDLOCK:
        case SQLITE_IOERR_CHECKRESERVEDLOCK:
        case SQLITE_IOERR_SHMLOCK:
            return true;
        default:
            break;
        }
    }

    return false;
}

foundation::Error makeSqliteError(sqlite3* database, const int resultCode)
{
    const int code =
        resultCode != 0 ? resultCode : (database != nullptr ? sqlite3_errcode(database) : SQLITE_OK);
    const int extendedCode =
        database != nullptr ? sqlite3_extended_errcode(database) : code;

    if (isSqliteLockContention(code, extendedCode))
    {
        return foundation::Error(foundation::ErrorCode::IOError, kIndexLockedMessage);
    }

    const char* message = database != nullptr ? sqlite3_errmsg(database) : "SQLite operation failed.";

    return foundation::Error(foundation::ErrorCode::IOError, message);
}

foundation::Result<bool> configureSqliteConnection(sqlite3* database)
{
    (void)sqlite3_busy_timeout(database, kSqliteBusyTimeoutMs);

    return foundation::Result<bool>(true);
}

} // namespace scope::storage
