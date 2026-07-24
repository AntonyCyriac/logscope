/**
 * @file index_store_factory_test.cpp
 */

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <optional>
#include <sqlite3.h>

#include "foundation/filesystem.hpp"
#include "index_fingerprint.hpp"
#include "index_store_factory.hpp"
#include "schema_version.hpp"
#include "sqlite_index_store.hpp"

using scope::analysis::LogFormat;
using scope::foundation::Path;
using scope::storage::IndexFingerprint;
using scope::storage::IndexMetadata;
using scope::storage::SqliteIndexStore;
using scope::storage::StorageConfig;
using scope::storage::createIndexStore;
using scope::storage::kIndexSchemaVersionCurrent;
using scope::storage::requiresSchemaRebuild;
using scope::storage::tryOpenReusableIndex;

namespace
{

Path writeTempSource(const std::string& name, const std::string& content)
{
    const Path sourcePath(name);
    std::ofstream output(sourcePath.string());
    output << content;
    output.close();

    return sourcePath;
}

Path uniqueDatabasePath(const std::string& suffix)
{
    const auto* testInfo = ::testing::UnitTest::GetInstance()->current_test_info();
    const std::string fileName = std::string(testInfo->test_suite_name()) + "_" + testInfo->name() + "_" + suffix +
                                 ".db";
    const Path databasePath(fileName);

    std::error_code error;
    std::filesystem::remove(databasePath.string(), error);

    return databasePath;
}

void cleanupPath(const Path& path)
{
    std::error_code error;
    std::filesystem::remove(path.string(), error);
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
    const Path databasePath = uniqueDatabasePath("rebuild");
    const Path sourcePath = writeTempSource("factory_test_v1_source.log", "sample line\n");
    const auto fingerprint = IndexFingerprint::compute(sourcePath);

    ASSERT_TRUE(fingerprint);

    IndexMetadata metadata;
    metadata.fingerprint = fingerprint->value();
    metadata.sourcePath = sourcePath;
    metadata.format = LogFormat::PlainText;

    ASSERT_TRUE(createV1Database(databasePath, metadata));

    StorageConfig config = StorageConfig::defaults();
    config.reuseIndex = true;
    config.indexPath = databasePath;

    const auto reused = tryOpenReusableIndex(config, *fingerprint, sourcePath);

    ASSERT_FALSE(reused);
    EXPECT_TRUE(requiresSchemaRebuild(reused.error()));

    const auto created = createIndexStore(config, *fingerprint, sourcePath, LogFormat::PlainText);

    ASSERT_TRUE(created);

    const auto schemaVersion = readMetaValue(databasePath, "schema_version");
    ASSERT_TRUE(schemaVersion.has_value());
    EXPECT_EQ(std::to_string(kIndexSchemaVersionCurrent), *schemaVersion);

    const auto opened = SqliteIndexStore::open(databasePath);
    ASSERT_TRUE(opened);

    cleanupPath(databasePath);
    cleanupPath(sourcePath);
}
