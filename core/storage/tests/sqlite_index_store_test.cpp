/**
 * @file sqlite_index_store_test.cpp
 */

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "index_fingerprint.hpp"
#include "sqlite_index_store.hpp"

using scope::storage::IndexFingerprint;

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
