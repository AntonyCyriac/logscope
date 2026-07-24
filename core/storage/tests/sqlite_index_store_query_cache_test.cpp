/**
 * @file sqlite_index_store_query_cache_test.cpp
 */

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>

#include "index_fingerprint.hpp"
#include "index_store_options.hpp"
#include "query_cache_codec.hpp"
#include "query_cache_key.hpp"
#include "sqlite_index_store.hpp"

using scope::analysis::DetectedLogLevel;
using scope::analysis::IndexedLine;
using scope::analysis::LogFormat;
using scope::foundation::Path;
using scope::storage::IndexFingerprint;
using scope::storage::IndexMetadata;
using scope::storage::IndexStoreOptions;
using scope::storage::SqliteIndexStore;
using scope::storage::computeQueryCacheKey;
using scope::storage::decodeLineNumbers;
using scope::storage::encodeLineNumbers;

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

Path writeTempSource(const Path& workspace, const std::string& content)
{
    const Path sourcePath = workspace.append("source.log");
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

IndexedLine makeLine(const std::uint64_t lineNumber, const DetectedLogLevel level)
{
    IndexedLine line;
    line.lineNumber = lineNumber;
    line.level = level;
    line.messageExcerpt = "sample";
    line.contentExcerpt = "sample";

    return line;
}

IndexMetadata makeMetadata(const Path& sourcePath)
{
    IndexMetadata metadata;
    metadata.fingerprint = IndexFingerprint::compute(sourcePath)->value();
    metadata.sourcePath = sourcePath;
    metadata.format = LogFormat::PlainText;

    return metadata;
}

std::shared_ptr<SqliteIndexStore> asSqliteStore(const scope::storage::IndexStorePtr& store)
{
    return std::static_pointer_cast<SqliteIndexStore>(store);
}

} // namespace

TEST(QueryCacheCodecTest, RoundTripsLineNumbers)
{
    const std::vector<std::uint64_t> lineNumbers = {1U, 42U, 999U};
    const std::string encoded = encodeLineNumbers(lineNumbers);
    const std::vector<std::uint64_t> decoded = decodeLineNumbers(encoded);

    ASSERT_EQ(lineNumbers.size(), decoded.size());

    for (std::size_t index = 0U; index < lineNumbers.size(); ++index)
    {
        EXPECT_EQ(lineNumbers[index], decoded[index]);
    }
}

TEST(QueryCacheKeyTest, ProducesStableDigest)
{
    const std::string first = computeQueryCacheKey("fp", "level == ERROR", 2);
    const std::string second = computeQueryCacheKey("fp", "level == ERROR", 2);

    EXPECT_EQ(first, second);
    EXPECT_NE(first, computeQueryCacheKey("fp", "level == INFO", 2));
}

TEST(SqliteIndexStoreQueryCacheTest, StoresResultsOnMissAndHitsOnRepeat)
{
    const Path workspace = testWorkspace();
    const Path databasePath = workspace.append("index.db");
    const Path sourcePath = writeTempSource(workspace, "error line\ninfo line\n");

    const auto created = SqliteIndexStore::create(databasePath, makeMetadata(sourcePath));
    ASSERT_TRUE(created);

    const auto store = asSqliteStore(*created);

    ASSERT_TRUE(store->appendLine(makeLine(1U, DetectedLogLevel::Error), "error line"));
    ASSERT_TRUE(store->appendLine(makeLine(2U, DetectedLogLevel::Info), "info line"));
    ASSERT_TRUE(store->finalize(2U));

    const std::string filter = "level == ERROR";
    const auto first = store->fetchLinesMatchingPushdown(filter, "level = 3");
    ASSERT_TRUE(first);
    EXPECT_FALSE(first->cacheHit);
    ASSERT_EQ(1U, first->lines.size());
    EXPECT_EQ(1U, store->queryCacheEntryCount());

    const auto second = store->fetchLinesMatchingPushdown(filter, "level = 3");
    ASSERT_TRUE(second);
    EXPECT_TRUE(second->cacheHit);
    ASSERT_EQ(1U, second->lines.size());
    EXPECT_EQ(1U, second->lines.front().lineNumber);

    cleanupWorkspace(workspace);
}

TEST(SqliteIndexStoreQueryCacheTest, DisabledCacheSkipsReadsAndWrites)
{
    const Path workspace = testWorkspace();
    const Path databasePath = workspace.append("index.db");
    const Path sourcePath = writeTempSource(workspace, "error line\n");

    IndexStoreOptions options;
    options.queryCacheEnabled = false;

    const auto created = SqliteIndexStore::create(databasePath, makeMetadata(sourcePath), options);
    ASSERT_TRUE(created);

    const auto store = asSqliteStore(*created);

    ASSERT_TRUE(store->appendLine(makeLine(1U, DetectedLogLevel::Error), "error line"));
    ASSERT_TRUE(store->finalize(1U));

    const auto first = store->fetchLinesMatchingPushdown("level == ERROR", "level = 3");
    ASSERT_TRUE(first);
    EXPECT_FALSE(first->cacheHit);
    EXPECT_EQ(0U, store->queryCacheEntryCount());

    const auto second = store->fetchLinesMatchingPushdown("level == ERROR", "level = 3");
    ASSERT_TRUE(second);
    EXPECT_FALSE(second->cacheHit);
    EXPECT_EQ(0U, store->queryCacheEntryCount());

    cleanupWorkspace(workspace);
}

TEST(SqliteIndexStoreQueryCacheTest, EvictsOldestEntriesWhenOverCapacity)
{
    const Path workspace = testWorkspace();
    const Path databasePath = workspace.append("index.db");
    const Path sourcePath = writeTempSource(workspace, "error line\ninfo line\nwarn line\n");

    IndexStoreOptions options;
    options.queryCacheMaxEntries = 2U;

    const auto created = SqliteIndexStore::create(databasePath, makeMetadata(sourcePath), options);
    ASSERT_TRUE(created);

    const auto store = asSqliteStore(*created);

    ASSERT_TRUE(store->appendLine(makeLine(1U, DetectedLogLevel::Error), "error line"));
    ASSERT_TRUE(store->appendLine(makeLine(2U, DetectedLogLevel::Info), "info line"));
    ASSERT_TRUE(store->appendLine(makeLine(3U, DetectedLogLevel::Warn), "warn line"));
    ASSERT_TRUE(store->finalize(3U));

    ASSERT_TRUE(store->fetchLinesMatchingPushdown("level == ERROR", "level = 3"));
    ASSERT_TRUE(store->fetchLinesMatchingPushdown("level == INFO", "level = 1"));
    ASSERT_TRUE(store->fetchLinesMatchingPushdown("level == WARNING", "level = 2"));

    EXPECT_LE(store->queryCacheEntryCount(), 2U);

    cleanupWorkspace(workspace);
}

TEST(SqliteIndexStoreQueryCacheTest, ClearsCacheWhenSourceTruncatedOnOpen)
{
    const Path workspace = testWorkspace();
    const Path databasePath = workspace.append("index.db");
    const Path sourcePath = writeTempSource(workspace, "error line\ninfo line\n");

    {
        const auto created = SqliteIndexStore::create(databasePath, makeMetadata(sourcePath));
        ASSERT_TRUE(created);
        const auto store = asSqliteStore(*created);
        ASSERT_TRUE(store->appendLine(makeLine(1U, DetectedLogLevel::Error), "error line"));
        ASSERT_TRUE(store->appendLine(makeLine(2U, DetectedLogLevel::Info), "info line"));
        ASSERT_TRUE(store->finalize(2U));
        ASSERT_TRUE(store->fetchLinesMatchingPushdown("level == ERROR", "level = 3"));
        ASSERT_EQ(1U, store->queryCacheEntryCount());
    }

    {
        std::ofstream truncated(sourcePath.string());
        truncated << "error line\n";
    }

    const auto opened = SqliteIndexStore::open(databasePath);
    ASSERT_TRUE(opened);
    EXPECT_EQ(0U, asSqliteStore(*opened)->queryCacheEntryCount());

    cleanupWorkspace(workspace);
}

TEST(SqliteIndexStoreQueryCacheTest, ClearQueryCacheRemovesAllEntries)
{
    const Path workspace = testWorkspace();
    const Path databasePath = workspace.append("index.db");
    const Path sourcePath = writeTempSource(workspace, "error line\n");

    const auto created = SqliteIndexStore::create(databasePath, makeMetadata(sourcePath));
    ASSERT_TRUE(created);
    const auto store = asSqliteStore(*created);
    ASSERT_TRUE(store->appendLine(makeLine(1U, DetectedLogLevel::Error), "error line"));
    ASSERT_TRUE(store->finalize(1U));
    ASSERT_TRUE(store->fetchLinesMatchingPushdown("level == ERROR", "level = 3"));
    ASSERT_EQ(1U, store->queryCacheEntryCount());

    store->clearQueryCache();
    EXPECT_EQ(0U, store->queryCacheEntryCount());

    cleanupWorkspace(workspace);
}
