/**
 * @file sqlite_index_store_test.cpp
 */

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <optional>
#include <sqlite3.h>

#include "index_fingerprint.hpp"
#include "schema_version.hpp"
#include "sqlite_index_store.hpp"
#include "foundation/filesystem.hpp"

using scope::storage::IndexFingerprint;
using scope::storage::kIndexSchemaVersionCurrent;
using scope::storage::requiresSchemaRebuild;

using scope::analysis::DetectedLogLevel;
using scope::analysis::IndexedLine;
using scope::analysis::LogFormat;
using scope::analysis::makeLineIndex;
using scope::foundation::Path;
using scope::storage::IndexMetadata;
using scope::storage::SqliteIndexStore;

namespace
{

IndexedLine makeLine(const std::uint64_t lineNumber, const DetectedLogLevel level, const std::string& message)
{
    IndexedLine line;
    line.lineNumber = lineNumber;
    line.level = level;
    line.messageExcerpt = message;
    line.contentExcerpt = message;

    return line;
}

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

bool tableExists(const Path& databasePath, const std::string& tableName)
{
    sqlite3* database = nullptr;

    if (sqlite3_open(databasePath.string().c_str(), &database) != SQLITE_OK)
    {
        if (database != nullptr)
        {
            sqlite3_close(database);
        }

        return false;
    }

    sqlite3_stmt* statement = nullptr;
    const char* sql = "SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = ? LIMIT 1;";

    if (sqlite3_prepare_v2(database, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        sqlite3_close(database);

        return false;
    }

    sqlite3_bind_text(statement, 1, tableName.c_str(), -1, SQLITE_TRANSIENT);

    const bool exists = sqlite3_step(statement) == SQLITE_ROW;

    sqlite3_finalize(statement);
    sqlite3_close(database);

    return exists;
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
CREATE INDEX idx_lines_level ON lines(level);
CREATE INDEX idx_lines_timestamp ON lines(timestamp_unix);
CREATE INDEX idx_lines_correlation ON lines(correlation_id);
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

IndexMetadata makeMetadata(const Path& sourcePath)
{
    const auto fingerprint = IndexFingerprint::compute(sourcePath);

    IndexMetadata metadata;
    metadata.fingerprint = fingerprint->value();
    metadata.sourcePath = sourcePath;
    metadata.format = LogFormat::PlainText;

    return metadata;
}

} // namespace

TEST(SqliteIndexStoreTest, AppendsAndFetchesLines)
{
    const Path databasePath = uniqueDatabasePath("append");
    const Path sourcePath = writeTempSource("storage_test_source.log", "error line\n");
    const auto fingerprint = IndexFingerprint::compute(sourcePath);

    ASSERT_TRUE(fingerprint);

    IndexMetadata metadata;
    metadata.fingerprint = fingerprint->value();
    metadata.sourcePath = sourcePath;
    metadata.format = LogFormat::PlainText;

    const auto created = SqliteIndexStore::create(databasePath, metadata);
    ASSERT_TRUE(created);

    ASSERT_TRUE((*created)->appendLine(makeLine(1U, DetectedLogLevel::Error, "Connection refused"), "full line"));
    ASSERT_TRUE((*created)->finalize(1U));

    const auto opened = SqliteIndexStore::open(databasePath);
    ASSERT_TRUE(opened);
    EXPECT_EQ(1U, (*opened)->storedLineCount());

    const auto lines = (*opened)->fetchAllLines();
    ASSERT_TRUE(lines);
    ASSERT_EQ(1U, lines->size());
    EXPECT_EQ(DetectedLogLevel::Error, lines->front().level);

    cleanupPath(databasePath);
    cleanupPath(sourcePath);
}

TEST(SqliteIndexStoreTest, FetchesFilteredLines)
{
    const Path databasePath = uniqueDatabasePath("filter");
    const Path sourcePath = writeTempSource("storage_test_filter_source.log", "error line\ninfo line\n");
    const auto fingerprint = IndexFingerprint::compute(sourcePath);

    ASSERT_TRUE(fingerprint);

    IndexMetadata metadata;
    metadata.fingerprint = fingerprint->value();
    metadata.sourcePath = sourcePath;
    metadata.format = LogFormat::PlainText;

    const auto created = SqliteIndexStore::create(databasePath, metadata);
    ASSERT_TRUE(created);

    ASSERT_TRUE((*created)->appendLine(makeLine(1U, DetectedLogLevel::Error, "Connection refused"), "line 1"));
    ASSERT_TRUE((*created)->appendLine(makeLine(2U, DetectedLogLevel::Info, "Started"), "line 2"));
    ASSERT_TRUE((*created)->finalize(2U));

    const auto lines = (*created)->fetchLinesWhere("level = 3");
    ASSERT_TRUE(lines);
    ASSERT_EQ(1U, lines->size());
    EXPECT_EQ(1U, lines->front().lineNumber);

    cleanupPath(databasePath);
    cleanupPath(sourcePath);
}

TEST(SqliteIndexStoreTest, PersistsMetadata)
{
    const Path databasePath = uniqueDatabasePath("metadata");
    const Path sourcePath = writeTempSource("storage_test_metadata_source.log", "sample\n");
    const auto fingerprint = IndexFingerprint::compute(sourcePath);

    ASSERT_TRUE(fingerprint);

    IndexMetadata metadata;
    metadata.fingerprint = fingerprint->value();
    metadata.sourcePath = sourcePath;
    metadata.format = LogFormat::PlainText;

    const auto created = SqliteIndexStore::create(databasePath, metadata);
    ASSERT_TRUE(created);
    ASSERT_TRUE((*created)->appendLine(makeLine(1U, DetectedLogLevel::Info, "hello"), "hello"));
    ASSERT_TRUE((*created)->finalize(1U));

    const auto opened = SqliteIndexStore::open(databasePath);
    ASSERT_TRUE(opened);
    EXPECT_EQ(fingerprint->value(), (*opened)->metadata().fingerprint);
    EXPECT_EQ(sourcePath.string(), (*opened)->metadata().sourcePath.string());

    cleanupPath(databasePath);
    cleanupPath(sourcePath);
}

TEST(SqliteIndexStoreTest, ReturnsLinesInOrder)
{
    const Path databasePath = uniqueDatabasePath("order");
    const Path sourcePath = writeTempSource("storage_test_order_source.log", "one\ntwo\nthree\n");
    const auto fingerprint = IndexFingerprint::compute(sourcePath);

    ASSERT_TRUE(fingerprint);

    IndexMetadata metadata;
    metadata.fingerprint = fingerprint->value();
    metadata.sourcePath = sourcePath;
    metadata.format = LogFormat::PlainText;

    const auto created = SqliteIndexStore::create(databasePath, metadata);
    ASSERT_TRUE(created);

    ASSERT_TRUE((*created)->appendLine(makeLine(1U, DetectedLogLevel::Info, "one"), "one"));
    ASSERT_TRUE((*created)->appendLine(makeLine(2U, DetectedLogLevel::Warn, "two"), "two"));
    ASSERT_TRUE((*created)->appendLine(makeLine(3U, DetectedLogLevel::Error, "three"), "three"));
    ASSERT_TRUE((*created)->finalize(3U));

    const auto lines = (*created)->fetchAllLines();
    ASSERT_TRUE(lines);
    ASSERT_EQ(3U, lines->size());
    EXPECT_EQ(1U, lines->at(0U).lineNumber);
    EXPECT_EQ(3U, lines->at(2U).lineNumber);

    cleanupPath(databasePath);
    cleanupPath(sourcePath);
}

TEST(SqliteIndexStoreTest, AppendsLargeBatchInOrder)
{
    constexpr std::size_t lineCount = 12000U;
    const Path databasePath = uniqueDatabasePath("large_batch");
    const Path sourcePath = writeTempSource("storage_test_large_batch_source.log", "sample\n");
    const auto fingerprint = IndexFingerprint::compute(sourcePath);

    ASSERT_TRUE(fingerprint);

    IndexMetadata metadata;
    metadata.fingerprint = fingerprint->value();
    metadata.sourcePath = sourcePath;
    metadata.format = LogFormat::PlainText;

    const auto created = SqliteIndexStore::create(databasePath, metadata);
    ASSERT_TRUE(created);

    for (std::size_t index = 0U; index < lineCount; ++index)
    {
        const std::string message = "line " + std::to_string(index);
        ASSERT_TRUE((*created)->appendLine(makeLine(static_cast<std::uint64_t>(index + 1U), DetectedLogLevel::Info,
                                                    message),
                                           message));
    }

    ASSERT_TRUE((*created)->finalize(lineCount));

    const auto opened = SqliteIndexStore::open(databasePath);
    ASSERT_TRUE(opened);
    EXPECT_EQ(lineCount, (*opened)->storedLineCount());

    const auto lines = (*opened)->fetchAllLines();
    ASSERT_TRUE(lines);
    ASSERT_EQ(lineCount, lines->size());
    EXPECT_EQ(1U, lines->front().lineNumber);
    EXPECT_EQ(lineCount, lines->back().lineNumber);

    cleanupPath(databasePath);
    cleanupPath(sourcePath);
}

TEST(SqliteIndexStoreTest, RejectsMissingDatabase)
{
    const auto opened = SqliteIndexStore::open(Path("storage_missing_database.db"));

    EXPECT_FALSE(opened);
}

TEST(SqliteIndexStoreTest, FreshCreateUsesSchemaV2)
{
    const Path databasePath = uniqueDatabasePath("schema_v2");
    const Path sourcePath = writeTempSource("storage_test_schema_v2_source.log", "sample\n");
    const auto metadata = makeMetadata(sourcePath);

    const auto created = SqliteIndexStore::create(databasePath, metadata);
    ASSERT_TRUE(created);
    ASSERT_TRUE((*created)->appendLine(makeLine(1U, DetectedLogLevel::Info, "hello"), "hello"));
    ASSERT_TRUE((*created)->finalize(1U));

    const auto schemaVersion = readMetaValue(databasePath, "schema_version");
    ASSERT_TRUE(schemaVersion.has_value());
    EXPECT_EQ(std::to_string(kIndexSchemaVersionCurrent), *schemaVersion);
    EXPECT_EQ("none", readMetaValue(databasePath, "content_compression"));
    EXPECT_TRUE(tableExists(databasePath, "line_json_fields"));
    EXPECT_TRUE(tableExists(databasePath, "query_cache"));
    EXPECT_EQ(kIndexSchemaVersionCurrent, SqliteIndexStore::currentSchemaVersion());

    cleanupPath(databasePath);
    cleanupPath(sourcePath);
}

TEST(SqliteIndexStoreTest, OpenV1IndexRequiresRebuild)
{
    const Path databasePath = uniqueDatabasePath("schema_v1");
    const Path sourcePath = writeTempSource("storage_test_schema_v1_source.log", "sample\n");
    const auto metadata = makeMetadata(sourcePath);

    ASSERT_TRUE(createV1Database(databasePath, metadata));

    const auto opened = SqliteIndexStore::open(databasePath);

    ASSERT_FALSE(opened);
    EXPECT_TRUE(requiresSchemaRebuild(opened.error()));
    EXPECT_NE(opened.error().message().find("requires rebuild from source"), std::string::npos);

    cleanupPath(databasePath);
    cleanupPath(sourcePath);
}

TEST(SqliteIndexStoreTest, RejectsCorruptDatabase)
{
    const Path databasePath = uniqueDatabasePath("corrupt");

    {
        std::ofstream output(databasePath.string(), std::ios::binary);
        output << "not a sqlite database";
    }

    const auto opened = SqliteIndexStore::open(databasePath);

    EXPECT_FALSE(opened);

    cleanupPath(databasePath);
}

TEST(SqliteIndexStoreTest, RejectsUnsupportedFutureSchema)
{
    const Path databasePath = uniqueDatabasePath("schema_future");
    const Path sourcePath = writeTempSource("storage_test_schema_future_source.log", "sample\n");
    const auto metadata = makeMetadata(sourcePath);

    const auto created = SqliteIndexStore::create(databasePath, metadata);
    ASSERT_TRUE(created);

    sqlite3* database = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_open(databasePath.string().c_str(), &database));

    sqlite3_stmt* statement = nullptr;
    ASSERT_EQ(SQLITE_OK,
              sqlite3_prepare_v2(database,
                                 "UPDATE meta SET value = '99' WHERE key = 'schema_version';", -1, &statement,
                                 nullptr));
    ASSERT_EQ(SQLITE_DONE, sqlite3_step(statement));
    sqlite3_finalize(statement);
    sqlite3_close(database);

    const auto opened = SqliteIndexStore::open(databasePath);

    ASSERT_FALSE(opened);
    EXPECT_NE(opened.error().message().find("Unsupported index schema version"), std::string::npos);
    EXPECT_FALSE(requiresSchemaRebuild(opened.error()));

    cleanupPath(databasePath);
    cleanupPath(sourcePath);
}

TEST(SqliteIndexStoreTest, FinalizePersistsSourceSnapshotMeta)
{
    const Path databasePath = uniqueDatabasePath("source_snapshot");
    const Path sourcePath = writeTempSource("storage_test_source_snapshot_source.log", "one\ntwo\n");
    const auto metadata = makeMetadata(sourcePath);

    const auto created = SqliteIndexStore::create(databasePath, metadata);
    ASSERT_TRUE(created);

    ASSERT_TRUE((*created)->appendLine(makeLine(1U, DetectedLogLevel::Info, "one"), "one"));
    ASSERT_TRUE((*created)->appendLine(makeLine(2U, DetectedLogLevel::Warn, "two"), "two"));
    ASSERT_TRUE((*created)->finalize(2U));

    const auto sizeResult = scope::foundation::FileSystem::fileSize(sourcePath);
    ASSERT_TRUE(sizeResult);

    const auto indexedLineCount = readMetaValue(databasePath, "indexed_line_count");
    const auto sourceSize = readMetaValue(databasePath, "source_size");
    const auto sourceMtime = readMetaValue(databasePath, "source_mtime");

    ASSERT_TRUE(indexedLineCount.has_value());
    ASSERT_TRUE(sourceSize.has_value());
    ASSERT_TRUE(sourceMtime.has_value());
    EXPECT_EQ("2", *indexedLineCount);
    EXPECT_EQ(std::to_string(*sizeResult), *sourceSize);
    EXPECT_FALSE(sourceMtime->empty());

    cleanupPath(databasePath);
    cleanupPath(sourcePath);
}
