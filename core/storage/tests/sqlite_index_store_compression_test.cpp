/**
 * @file sqlite_index_store_compression_test.cpp
 */

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <optional>
#include <sqlite3.h>
#include <string>

#include "index_fingerprint.hpp"
#include "index_store_options.hpp"
#include "content_codec.hpp"
#include "sqlite_index_store.hpp"

using scope::analysis::DetectedLogLevel;
using scope::analysis::IndexedLine;
using scope::analysis::LogFormat;
using scope::foundation::Path;
using scope::storage::IndexFingerprint;
using scope::storage::IndexMetadata;
using scope::storage::IndexStoreOptions;
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

Path testWorkspace()
{
    const auto* testInfo = ::testing::UnitTest::GetInstance()->current_test_info();
    const Path workspace(std::string(testInfo->test_suite_name()) + "_" + testInfo->name() + "_ws");

    std::error_code error;
    std::filesystem::remove_all(workspace.string(), error);
    std::filesystem::create_directories(workspace.string(), error);

    return workspace;
}

Path uniqueSourcePath(const Path& workspace, const std::string& suffix)
{
    return workspace.append(suffix + ".log");
}

Path writeTempSource(const Path& sourcePath, const std::string& content)
{
    std::ofstream output(sourcePath.string());
    output << content;
    output.close();

    return sourcePath;
}

Path uniqueDatabasePath(const Path& workspace, const std::string& suffix)
{
    return workspace.append(suffix + ".db");
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

std::optional<std::string> contentStorageType(const Path& databasePath, const std::uint64_t lineNumber)
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
    const char* sql = "SELECT typeof(content) FROM lines WHERE line_number = ? LIMIT 1;";

    if (sqlite3_prepare_v2(database, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        sqlite3_close(database);

        return std::nullopt;
    }

    sqlite3_bind_int64(statement, 1, static_cast<sqlite3_int64>(lineNumber));

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

IndexMetadata makeMetadata(const Path& sourcePath)
{
    const auto fingerprint = IndexFingerprint::compute(sourcePath);

    IndexMetadata metadata;
    metadata.fingerprint = fingerprint->value();
    metadata.sourcePath = sourcePath;
    metadata.format = LogFormat::PlainText;

    return metadata;
}

std::string repeatCharacter(const char character, const std::size_t count)
{
    return std::string(count, character);
}

std::string zlibExpandedContent()
{
    const std::string repetitive = repeatCharacter('x', 400U);
    const auto compressed = scope::storage::compressZlib(repetitive);
    EXPECT_TRUE(compressed);

    return *compressed;
}

std::uintmax_t databaseFileSize(const Path& databasePath)
{
    return std::filesystem::file_size(databasePath.string());
}

std::string typicalShortLogLine(const std::uint64_t sequence)
{
    return "2024-01-15T10:00:00.000 INFO [service] request_id=" + std::to_string(sequence) +
           " completed in 42ms";
}

} // namespace

TEST(SqliteIndexStoreCompressionTest, StoresCompressedContentWhenEnabled)
{
    const Path workspace = testWorkspace();
    const Path databasePath = uniqueDatabasePath(workspace, "compressed");
    const Path sourcePath = writeTempSource(uniqueSourcePath(workspace, "source"), "sample\n");
    const auto metadata = makeMetadata(sourcePath);

    IndexStoreOptions options;
    options.compressContent = true;
    options.compressThresholdBytes = 16U;

    const auto created = SqliteIndexStore::create(databasePath, metadata, options);
    ASSERT_TRUE(created);

    const std::string longLine = repeatCharacter('x', 400U);
    ASSERT_TRUE((*created)->appendLine(makeLine(1U, DetectedLogLevel::Info, longLine), longLine));
    ASSERT_TRUE((*created)->finalize(1U));

    EXPECT_EQ("zlib", readMetaValue(databasePath, "content_compression"));
    ASSERT_TRUE(contentStorageType(databasePath, 1U).has_value());
    EXPECT_EQ("blob", *contentStorageType(databasePath, 1U));

    cleanupWorkspace(workspace);
}

TEST(SqliteIndexStoreCompressionTest, StoresPlainTextWhenDisabled)
{
    const Path workspace = testWorkspace();
    const Path databasePath = uniqueDatabasePath(workspace, "plain");
    const Path sourcePath = writeTempSource(uniqueSourcePath(workspace, "source"), "sample\n");
    const auto metadata = makeMetadata(sourcePath);

    IndexStoreOptions options;
    options.compressContent = false;

    const auto created = SqliteIndexStore::create(databasePath, metadata, options);
    ASSERT_TRUE(created);

    const std::string longLine = repeatCharacter('y', 400U);
    ASSERT_TRUE((*created)->appendLine(makeLine(1U, DetectedLogLevel::Info, longLine), longLine));
    ASSERT_TRUE((*created)->finalize(1U));

    EXPECT_EQ("none", readMetaValue(databasePath, "content_compression"));
    ASSERT_TRUE(contentStorageType(databasePath, 1U).has_value());
    EXPECT_EQ("text", *contentStorageType(databasePath, 1U));

    cleanupWorkspace(workspace);
}

TEST(SqliteIndexStoreCompressionTest, SkipsCompressionBelowThreshold)
{
    const Path workspace = testWorkspace();
    const Path databasePath = uniqueDatabasePath(workspace, "threshold");
    const Path sourcePath = writeTempSource(uniqueSourcePath(workspace, "source"), "sample\n");
    const auto metadata = makeMetadata(sourcePath);

    IndexStoreOptions options;
    options.compressContent = true;
    options.compressThresholdBytes = 256U;

    const auto created = SqliteIndexStore::create(databasePath, metadata, options);
    ASSERT_TRUE(created);

    const std::string shortLine = repeatCharacter('z', 64U);
    ASSERT_TRUE((*created)->appendLine(makeLine(1U, DetectedLogLevel::Info, shortLine), shortLine));
    ASSERT_TRUE((*created)->finalize(1U));

    EXPECT_EQ("zlib", readMetaValue(databasePath, "content_compression"));
    ASSERT_TRUE(contentStorageType(databasePath, 1U).has_value());
    EXPECT_EQ("text", *contentStorageType(databasePath, 1U));

    cleanupWorkspace(workspace);
}

TEST(SqliteIndexStoreCompressionTest, SkipsCompressionWhenCompressedBlobWouldBeLarger)
{
    const Path workspace = testWorkspace();
    const Path databasePath = uniqueDatabasePath(workspace, "skip_expansion");
    const Path sourcePath = writeTempSource(uniqueSourcePath(workspace, "source"), "sample\n");
    const auto metadata = makeMetadata(sourcePath);

    const std::string line = zlibExpandedContent();
    const auto compressed = scope::storage::compressZlib(line);
    ASSERT_TRUE(compressed);
    ASSERT_GT(compressed->size(), line.size());

    IndexStoreOptions options;
    options.compressContent = true;
    options.compressThresholdBytes = 16U;

    const auto created = SqliteIndexStore::create(databasePath, metadata, options);
    ASSERT_TRUE(created);
    ASSERT_TRUE((*created)->appendLine(makeLine(1U, DetectedLogLevel::Info, line), line));
    ASSERT_TRUE((*created)->finalize(1U));

    EXPECT_EQ("zlib", readMetaValue(databasePath, "content_compression"));
    ASSERT_TRUE(contentStorageType(databasePath, 1U).has_value());
    EXPECT_EQ("text", *contentStorageType(databasePath, 1U));

    cleanupWorkspace(workspace);
}

TEST(SqliteIndexStoreCompressionTest, CompressedIndexNotLargerThanPlainForTypicalShortLines)
{
    const Path workspace = testWorkspace();
    const Path sourcePath = uniqueSourcePath(workspace, "short_lines");
    std::string sourceContent;

    for (std::uint64_t lineNumber = 1U; lineNumber <= 500U; ++lineNumber)
    {
        sourceContent += typicalShortLogLine(lineNumber) + '\n';
    }

    writeTempSource(sourcePath, sourceContent);
    const auto metadata = makeMetadata(sourcePath);

    const Path plainDatabasePath = uniqueDatabasePath(workspace, "plain_short");
    IndexStoreOptions plainOptions;
    plainOptions.compressContent = false;

    const auto plainStore = SqliteIndexStore::create(plainDatabasePath, metadata, plainOptions);
    ASSERT_TRUE(plainStore);

    for (std::uint64_t lineNumber = 1U; lineNumber <= 500U; ++lineNumber)
    {
        const std::string line = typicalShortLogLine(lineNumber);
        ASSERT_TRUE((*plainStore)->appendLine(makeLine(lineNumber, DetectedLogLevel::Info, line), line));
    }

    ASSERT_TRUE((*plainStore)->finalize(500U));

    const Path compressedDatabasePath = uniqueDatabasePath(workspace, "compressed_short");
    IndexStoreOptions compressedOptions;
    compressedOptions.compressContent = true;
    compressedOptions.compressThresholdBytes = 16U;

    const auto compressedStore = SqliteIndexStore::create(compressedDatabasePath, metadata, compressedOptions);
    ASSERT_TRUE(compressedStore);

    for (std::uint64_t lineNumber = 1U; lineNumber <= 500U; ++lineNumber)
    {
        const std::string line = typicalShortLogLine(lineNumber);
        ASSERT_TRUE((*compressedStore)->appendLine(makeLine(lineNumber, DetectedLogLevel::Info, line), line));
    }

    ASSERT_TRUE((*compressedStore)->finalize(500U));

    EXPECT_LE(databaseFileSize(compressedDatabasePath), databaseFileSize(plainDatabasePath));

    cleanupWorkspace(workspace);
}

TEST(SqliteIndexStoreCompressionTest, FetchesDecompressedContentRoundTrip)
{
    const Path workspace = testWorkspace();
    const Path databasePath = uniqueDatabasePath(workspace, "roundtrip");
    const Path sourcePath = writeTempSource(uniqueSourcePath(workspace, "source"), "sample\n");
    const auto metadata = makeMetadata(sourcePath);

    IndexStoreOptions options;
    options.compressContent = true;
    options.compressThresholdBytes = 32U;

    const std::string longLine = repeatCharacter('a', 512U) + " unique marker";

    const auto created = SqliteIndexStore::create(databasePath, metadata, options);
    ASSERT_TRUE(created);
    ASSERT_TRUE((*created)->appendLine(makeLine(1U, DetectedLogLevel::Error, longLine), longLine));
    ASSERT_TRUE((*created)->finalize(1U));

    const auto opened = SqliteIndexStore::open(databasePath);
    ASSERT_TRUE(opened);

    const auto lines = (*opened)->fetchAllLines();
    ASSERT_TRUE(lines);
    ASSERT_EQ(1U, lines->size());
    EXPECT_EQ(longLine, lines->front().contentExcerpt);

    cleanupWorkspace(workspace);
}

TEST(SqliteIndexStoreCompressionTest, RejectsCorruptCompressedBlob)
{
    const Path workspace = testWorkspace();
    const Path databasePath = uniqueDatabasePath(workspace, "corrupt_blob");
    const Path sourcePath = writeTempSource(uniqueSourcePath(workspace, "source"), "sample\n");
    const auto metadata = makeMetadata(sourcePath);

    IndexStoreOptions options;
    options.compressContent = true;
    options.compressThresholdBytes = 16U;

    const std::string longLine = repeatCharacter('b', 400U);

    const auto created = SqliteIndexStore::create(databasePath, metadata, options);
    ASSERT_TRUE(created);
    ASSERT_TRUE((*created)->appendLine(makeLine(1U, DetectedLogLevel::Error, longLine), longLine));
    ASSERT_TRUE((*created)->finalize(1U));

    sqlite3* database = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_open(databasePath.string().c_str(), &database));

    sqlite3_stmt* statement = nullptr;
    ASSERT_EQ(SQLITE_OK,
              sqlite3_prepare_v2(database, "UPDATE lines SET content = ? WHERE line_number = 1;", -1, &statement,
                                 nullptr));

    const unsigned char corruptBytes[] = {0x78U, 0x9CU, 0x01U, 0x02U, 0x03U};
    sqlite3_bind_blob(statement, 1, corruptBytes, static_cast<int>(sizeof(corruptBytes)), SQLITE_TRANSIENT);
    ASSERT_EQ(SQLITE_DONE, sqlite3_step(statement));
    sqlite3_finalize(statement);
    sqlite3_close(database);

    const auto opened = SqliteIndexStore::open(databasePath);
    ASSERT_TRUE(opened);

    const auto lines = (*opened)->fetchAllLines();
    ASSERT_FALSE(lines);
    EXPECT_EQ(scope::foundation::ErrorCode::ParseError, lines.error().code());

    cleanupWorkspace(workspace);
}

TEST(SqliteIndexStoreCompressionTest, AdaptiveShortCircuitDisablesFutileCompression)
{
    const Path workspace = testWorkspace();
    const Path sourcePath = uniqueSourcePath(workspace, "adaptive_short");
    std::string sourceContent;

    for (std::uint64_t lineNumber = 1U; lineNumber <= 64U; ++lineNumber)
    {
        sourceContent += typicalShortLogLine(lineNumber) + '\n';
    }

    writeTempSource(sourcePath, sourceContent);
    const auto metadata = makeMetadata(sourcePath);

    const Path databasePath = uniqueDatabasePath(workspace, "adaptive_short");
    IndexStoreOptions options;
    options.compressContent = true;
    options.compressThresholdBytes = 1U;

    const auto created = SqliteIndexStore::create(databasePath, metadata, options);
    ASSERT_TRUE(created);

    for (std::uint64_t lineNumber = 1U; lineNumber <= 64U; ++lineNumber)
    {
        const std::string line = typicalShortLogLine(lineNumber);
        ASSERT_TRUE((*created)->appendLine(makeLine(lineNumber, DetectedLogLevel::Info, line), line));
    }

    ASSERT_TRUE((*created)->finalize(64U));

    const auto attempts = readMetaValue(databasePath, "content_compression_attempts");
    const auto wins = readMetaValue(databasePath, "content_compression_wins");
    const auto adaptiveDisabled = readMetaValue(databasePath, "content_compression_adaptive_disabled");

    ASSERT_TRUE(attempts.has_value());
    ASSERT_TRUE(wins.has_value());
    ASSERT_TRUE(adaptiveDisabled.has_value());
    EXPECT_EQ("32", *attempts);
    EXPECT_EQ("0", *wins);
    EXPECT_EQ("true", *adaptiveDisabled);

    cleanupWorkspace(workspace);
}
