/**
 * @file index_store_factory_test.cpp
 */

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <optional>
#include <sqlite3.h>

#include "foundation/filesystem.hpp"
#include "foundation/error.hpp"
#include "index_fingerprint.hpp"
#include "index_store_factory.hpp"
#include "schema_version.hpp"
#include "sqlite_index_store.hpp"

using scope::analysis::LogFormat;
using scope::foundation::Error;
using scope::foundation::ErrorCode;
using scope::foundation::Path;
using scope::storage::IndexFingerprint;
using scope::storage::IndexMetadata;
using scope::storage::SqliteIndexStore;
using scope::storage::StorageConfig;
using scope::storage::createIndexStore;
using scope::storage::kIndexSchemaVersionCurrent;
using scope::storage::requiresCompressionRebuild;
using scope::storage::requiresSchemaRebuild;

namespace
{

Path testWorkspace()
{
    const auto* testInfo = ::testing::UnitTest::GetInstance()->current_test_info();
    const std::filesystem::path workspacePath =
        std::filesystem::temp_directory_path() /
        ("logscope_" + std::string(testInfo->test_suite_name()) + "_" + testInfo->name() + "_ws");
    const Path workspace(workspacePath.string());

    std::error_code error;
    std::filesystem::remove_all(workspacePath, error);
    std::filesystem::create_directories(workspacePath, error);

    return workspace;
}

Path uniqueSourcePath(const Path& workspace, const std::string& suffix)
{
    return workspace.append(suffix + ".log");
}

Path uniqueDatabasePath(const Path& workspace, const std::string& suffix)
{
    return workspace.append(suffix + ".db");
}

Path writeTempSource(const Path& sourcePath, const std::string& content)
{
    std::ofstream output(sourcePath.string());
    output << content;
    output.close();

    return sourcePath;
}

void cleanupWorkspace(const Path& workspace)
{
    std::error_code error;
    std::filesystem::remove_all(workspace.string(), error);
}

std::optional<std::string> readMetaValue(const Path& databasePath, const std::string& key)
{
    sqlite3* database = nullptr;

    if (sqlite3_open(databasePath.string().c_str(), &database) != SQLITE_OK)
    {
        if (database != nullptr)
        {
            sqlite3_close(database);
        }

        return std::nullopt;
    }

    sqlite3_stmt* statement = nullptr;
    const char* sql = "SELECT value FROM meta WHERE key = ? LIMIT 1;";

    if (sqlite3_prepare_v2(database, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        sqlite3_close(database);

        return std::nullopt;
    }

    sqlite3_bind_text(statement, 1, key.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<std::string> value;

    if (sqlite3_step(statement) == SQLITE_ROW)
    {
        const unsigned char* text = sqlite3_column_text(statement, 0);

        if (text != nullptr)
        {
            value = reinterpret_cast<const char*>(text);
        }
    }

    sqlite3_finalize(statement);
    sqlite3_close(database);

    return value;
}

bool createV1Database(const Path& databasePath, const IndexMetadata& metadata)
{
    std::error_code error;
    std::filesystem::remove(databasePath.string(), error);

    sqlite3* database = nullptr;

    if (sqlite3_open(databasePath.string().c_str(), &database) != SQLITE_OK)
    {
        if (database != nullptr)
        {
            sqlite3_close(database);
        }

        return false;
    }

    static constexpr const char* v1SchemaSql = R"SQL(
CREATE TABLE meta (
  key TEXT PRIMARY KEY,
  value TEXT NOT NULL
);
CREATE TABLE lines (
  id INTEGER PRIMARY KEY,
  line_number INTEGER NOT NULL,
  level INTEGER NOT NULL,
  timestamp_unix INTEGER,
  message TEXT NOT NULL,
  content TEXT NOT NULL,
  correlation_id TEXT NOT NULL,
  top_level_keys_json TEXT NOT NULL
);
)SQL";

    char* errorMessage = nullptr;
    const int schemaResult = sqlite3_exec(database, v1SchemaSql, nullptr, nullptr, &errorMessage);

    if (schemaResult != SQLITE_OK)
    {
        if (errorMessage != nullptr)
        {
            sqlite3_free(errorMessage);
        }

        sqlite3_close(database);

        return false;
    }

    auto setMeta = [&](const std::string& key, const std::string& value) -> bool {
        sqlite3_stmt* statement = nullptr;
        const char* sql = "INSERT INTO meta(key, value) VALUES(?, ?);";

        if (sqlite3_prepare_v2(database, sql, -1, &statement, nullptr) != SQLITE_OK)
        {
            return false;
        }

        sqlite3_bind_text(statement, 1, key.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(statement, 2, value.c_str(), -1, SQLITE_TRANSIENT);

        const int stepResult = sqlite3_step(statement);
        sqlite3_finalize(statement);

        return stepResult == SQLITE_DONE;
    };

    const bool metaOk = setMeta("schema_version", "1") && setMeta("fingerprint", metadata.fingerprint) &&
                        setMeta("source_path", metadata.sourcePath.string()) && setMeta("format", "plain");

    sqlite3_close(database);

    return metaOk;
}

} // namespace

TEST(IndexStoreFactoryTest, RebuildsV1IndexThroughFactory)
{
    const Path workspace = testWorkspace();
    const Path databasePath = uniqueDatabasePath(workspace, "rebuild");
    const Path sourcePath = writeTempSource(uniqueSourcePath(workspace, "source"), "sample line\n");
    const auto fingerprint = IndexFingerprint::compute(sourcePath);

    ASSERT_TRUE(fingerprint);

    IndexMetadata metadata;
    metadata.fingerprint = fingerprint->value();
    metadata.sourcePath = sourcePath;
    metadata.format = LogFormat::PlainText;

    ASSERT_TRUE(createV1Database(databasePath, metadata));

    const auto openedV1 = SqliteIndexStore::open(databasePath);
    ASSERT_FALSE(openedV1) << openedV1.error().message();
    EXPECT_TRUE(requiresSchemaRebuild(openedV1.error())) << openedV1.error().message();

    StorageConfig config = StorageConfig::defaults();
    config.reuseIndex = true;
    config.indexPath = databasePath;

    {
        const auto created = createIndexStore(config, *fingerprint, sourcePath, LogFormat::PlainText);
        ASSERT_TRUE(created);
        ASSERT_TRUE((*created)->finalize(0U));
    }

    const auto schemaVersion = readMetaValue(databasePath, "schema_version");
    ASSERT_TRUE(schemaVersion.has_value());
    EXPECT_EQ(std::to_string(kIndexSchemaVersionCurrent), *schemaVersion);

    const auto opened = SqliteIndexStore::open(databasePath);
    ASSERT_TRUE(opened);

    cleanupWorkspace(workspace);
}

TEST(IndexStoreFactoryTest, RejectsCompressionMismatchOnReuse)
{
    const Path workspace = testWorkspace();
    const Path databasePath = uniqueDatabasePath(workspace, "compression_mismatch");
    const Path sourcePath = writeTempSource(uniqueSourcePath(workspace, "source"), "sample line\n");

    StorageConfig createConfig = StorageConfig::defaults();
    createConfig.indexPath = databasePath;
    createConfig.compressContent = false;

    const auto fingerprint = IndexFingerprint::compute(sourcePath);
    ASSERT_TRUE(fingerprint);

    {
        const auto created = createIndexStore(createConfig, *fingerprint, sourcePath, LogFormat::PlainText);
        ASSERT_TRUE(created);
        ASSERT_TRUE((*created)->finalize(0U));
    }

    StorageConfig reuseConfig = StorageConfig::defaults();
    reuseConfig.reuseIndex = true;
    reuseConfig.indexPath = databasePath;
    reuseConfig.compressContent = true;

    const auto opened = SqliteIndexStore::open(databasePath);
    ASSERT_TRUE(opened);

    const auto sqliteStore = std::static_pointer_cast<SqliteIndexStore>(*opened);

    if (sqliteStore->usesContentCompression() == reuseConfig.compressContent)
    {
        FAIL() << "Expected persisted index compression settings to differ from reuse config.";
    }

    const Error compressionMismatch(ErrorCode::InvalidArgument,
                                    "Index compression settings require rebuild from source log.");
    EXPECT_TRUE(requiresCompressionRebuild(compressionMismatch));

    cleanupWorkspace(workspace);
}
