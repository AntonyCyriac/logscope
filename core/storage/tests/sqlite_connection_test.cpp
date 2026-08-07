/**
 * @file sqlite_connection_test.cpp
 */

#include <gtest/gtest.h>

#include <filesystem>
#include <sqlite3.h>

#include "foundation/filesystem.hpp"
#include "gtest_temp_path.hpp"
#include "sqlite_connection.hpp"

using scope::foundation::ErrorCode;
using scope::foundation::Path;
using scope::storage::configureSqliteConnection;
using scope::storage::isSqliteLockContention;
using scope::storage::kSqliteBusyTimeoutMs;
using scope::storage::makeSqliteError;

namespace
{

Path uniqueDatabasePath()
{
    const auto* testInfo = ::testing::UnitTest::GetInstance()->current_test_info();
    const std::string fileName =
        std::string(testInfo->test_suite_name()) + "_" + testInfo->name() + ".db";

    return Path(fileName);
}

} // namespace

TEST(SqliteConnectionTest, ConfiguresBusyTimeout)
{
    const Path databasePath = uniqueDatabasePath();
    sqlite3* database = nullptr;

    ASSERT_EQ(SQLITE_OK, sqlite3_open(databasePath.string().c_str(), &database));

    const auto configured = configureSqliteConnection(database);

    ASSERT_TRUE(configured);

    sqlite3_stmt* statement = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_prepare_v2(database, "PRAGMA busy_timeout;", -1, &statement, nullptr));
    ASSERT_EQ(SQLITE_ROW, sqlite3_step(statement));
    EXPECT_EQ(kSqliteBusyTimeoutMs, sqlite3_column_int(statement, 0));
    sqlite3_finalize(statement);

    sqlite3_close(database);
    std::error_code error;
    std::filesystem::remove(databasePath.string(), error);
}

TEST(SqliteConnectionTest, DetectsLockContentionCodes)
{
    EXPECT_TRUE(isSqliteLockContention(SQLITE_BUSY, SQLITE_BUSY));
    EXPECT_TRUE(isSqliteLockContention(SQLITE_LOCKED, SQLITE_LOCKED));
    EXPECT_TRUE(isSqliteLockContention(SQLITE_IOERR, SQLITE_IOERR_LOCK));
    EXPECT_TRUE(isSqliteLockContention(SQLITE_IOERR, SQLITE_IOERR_BLOCKED));
    EXPECT_TRUE(isSqliteLockContention(SQLITE_IOERR, SQLITE_IOERR_RDLOCK));
    EXPECT_TRUE(isSqliteLockContention(SQLITE_IOERR, SQLITE_IOERR_CHECKRESERVEDLOCK));
    EXPECT_TRUE(isSqliteLockContention(SQLITE_IOERR, SQLITE_IOERR_SHMLOCK));
    EXPECT_FALSE(isSqliteLockContention(SQLITE_IOERR, SQLITE_IOERR_READ));
    EXPECT_FALSE(isSqliteLockContention(SQLITE_CORRUPT, SQLITE_CORRUPT));
}

TEST(SqliteConnectionTest, MapsLockContentionToClearMessage)
{
    const auto busyError = makeSqliteError(nullptr, SQLITE_BUSY);
    EXPECT_EQ(ErrorCode::IOError, busyError.code());
    EXPECT_EQ("Index is locked by another process.", busyError.message());

    const auto lockedError = makeSqliteError(nullptr, SQLITE_LOCKED);
    EXPECT_EQ(ErrorCode::IOError, lockedError.code());
    EXPECT_EQ("Index is locked by another process.", lockedError.message());
}
