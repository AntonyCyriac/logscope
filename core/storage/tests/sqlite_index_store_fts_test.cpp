/**
 * @file sqlite_index_store_fts_test.cpp
 */

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <optional>
#include <sqlite3.h>

#include "index_fingerprint.hpp"
#include "query_evaluator.hpp"
#include "query_parser.hpp"
#include "query_planner.hpp"
#include "sqlite_index_store.hpp"

using scope::analysis::DetectedLogLevel;
using scope::analysis::IndexedLine;
using scope::analysis::LogFormat;
using scope::foundation::Path;
using scope::query::QueryEvaluator;
using scope::query::parseFilterQuery;
using scope::storage::IndexFingerprint;
using scope::storage::IndexMetadata;
using scope::storage::SqliteIndexStore;
using scope::storage::planQueryPushdown;

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

std::optional<std::size_t> countFtsRows(const Path& databasePath)
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

    if (sqlite3_prepare_v2(database, "SELECT COUNT(*) FROM lines_fts;", -1, &statement, nullptr) != SQLITE_OK)
    {
        sqlite3_close(database);

        return std::nullopt;
    }

    std::optional<std::size_t> count;

    if (sqlite3_step(statement) == SQLITE_ROW)
    {
        count = static_cast<std::size_t>(sqlite3_column_int64(statement, 0));
    }

    sqlite3_finalize(statement);
    sqlite3_close(database);

    return count;
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

TEST(SqliteIndexStoreFtsTest, PopulatesFtsTableOnAppend)
{
    const Path workspace = testWorkspace();
    const Path sourcePath = writeTempSource(workspace, "fts source\n");
    const Path databasePath = workspace.append("index.db");
    const IndexMetadata metadata = makeMetadata(sourcePath);

    const auto created = SqliteIndexStore::create(databasePath, metadata);
    ASSERT_TRUE(created);

    ASSERT_TRUE((*created)->appendLine(makeLine(1U, DetectedLogLevel::Error, "connection timeout"), "connection timeout"));
    ASSERT_TRUE((*created)->appendLine(makeLine(2U, DetectedLogLevel::Info, "healthy"), "healthy"));
    ASSERT_TRUE((*created)->finalize(2U));

    const auto ftsCount = countFtsRows(databasePath);
    ASSERT_TRUE(ftsCount.has_value());
    EXPECT_EQ(2U, *ftsCount);

    cleanupWorkspace(workspace);
}

TEST(SqliteIndexStoreFtsTest, ContainsPushdownUsesFtsMatch)
{
    const Path workspace = testWorkspace();
    const Path sourcePath = writeTempSource(workspace, "fts source\n");
    const Path databasePath = workspace.append("index.db");
    const IndexMetadata metadata = makeMetadata(sourcePath);

    const auto created = SqliteIndexStore::create(databasePath, metadata);
    ASSERT_TRUE(created);

    ASSERT_TRUE((*created)->appendLine(makeLine(1U, DetectedLogLevel::Error, "connection timeout"), "connection timeout"));
    ASSERT_TRUE((*created)->appendLine(makeLine(2U, DetectedLogLevel::Info, "healthy"), "healthy"));
    ASSERT_TRUE((*created)->finalize(2U));

    const auto parsed = parseFilterQuery(R"(contains(message, "timeout"))");
    ASSERT_TRUE(parsed);

    const auto plan = planQueryPushdown(*parsed);
    ASSERT_TRUE(plan.has_value());

    const auto opened = SqliteIndexStore::open(databasePath);
    ASSERT_TRUE(opened);

    const auto matches = (*opened)->fetchLinesWhere(plan->sqlWhere);
    ASSERT_TRUE(matches);
    ASSERT_EQ(1U, matches->size());
    EXPECT_EQ(1U, (*matches)[0].lineNumber);
    EXPECT_NE(std::string::npos, (*matches)[0].messageExcerpt.find("timeout"));

    cleanupWorkspace(workspace);
}

TEST(SqliteIndexStoreFtsTest, FtsSearchReturnsMatchingLines)
{
    const Path workspace = testWorkspace();
    const Path sourcePath = writeTempSource(workspace, "fts source\n");
    const Path databasePath = workspace.append("index.db");
    const IndexMetadata metadata = makeMetadata(sourcePath);

    const auto created = SqliteIndexStore::create(databasePath, metadata);
    ASSERT_TRUE(created);

    ASSERT_TRUE((*created)->appendLine(makeLine(1U, DetectedLogLevel::Error, "connection timeout"), "connection timeout"));
    ASSERT_TRUE((*created)->appendLine(makeLine(2U, DetectedLogLevel::Info, "healthy"), "healthy"));
    ASSERT_TRUE((*created)->finalize(2U));

    const auto opened = SqliteIndexStore::open(databasePath);
    ASSERT_TRUE(opened);

    const auto sqliteStore = std::static_pointer_cast<SqliteIndexStore>(*opened);
    const auto matches = sqliteStore->fetchLinesMatchingFts("timeout");
    ASSERT_TRUE(matches);
    ASSERT_EQ(1U, matches->size());
    EXPECT_EQ(1U, (*matches)[0].lineNumber);

    cleanupWorkspace(workspace);
}

TEST(SqliteIndexStoreFtsTest, BackfillsFtsOnOpenWhenMissing)
{
    const Path workspace = testWorkspace();
    const Path sourcePath = writeTempSource(workspace, "fts source\n");
    const Path databasePath = workspace.append("index.db");
    const IndexMetadata metadata = makeMetadata(sourcePath);

    {
        const auto created = SqliteIndexStore::create(databasePath, metadata);
        ASSERT_TRUE(created);

        ASSERT_TRUE(
            (*created)->appendLine(makeLine(1U, DetectedLogLevel::Error, "connection timeout"), "connection timeout"));
        ASSERT_TRUE((*created)->finalize(1U));
    }

    sqlite3* database = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_open(databasePath.string().c_str(), &database));
    ASSERT_EQ(SQLITE_OK, sqlite3_exec(database, "DELETE FROM lines_fts;", nullptr, nullptr, nullptr));
    sqlite3_close(database);

    const auto opened = SqliteIndexStore::open(databasePath);
    ASSERT_TRUE(opened);

    const auto ftsCount = countFtsRows(databasePath);
    ASSERT_TRUE(ftsCount.has_value());
    EXPECT_EQ(1U, *ftsCount);

    const auto sqliteStore = std::static_pointer_cast<SqliteIndexStore>(*opened);
    const auto matches = sqliteStore->fetchLinesMatchingFts("timeout");
    ASSERT_TRUE(matches);
    ASSERT_EQ(1U, matches->size());

    cleanupWorkspace(workspace);
}

TEST(SqliteIndexStoreFtsTest, ContainsCjkUsesEvaluatorFallbackParity)
{
    const Path workspace = testWorkspace();
    const Path sourcePath = writeTempSource(workspace, "fts source\n");
    const Path databasePath = workspace.append("index.db");
    const IndexMetadata metadata = makeMetadata(sourcePath);

    const auto created = SqliteIndexStore::create(databasePath, metadata);
    ASSERT_TRUE(created);

    ASSERT_TRUE((*created)->appendLine(makeLine(1U, DetectedLogLevel::Info, "hello-world"), "hello-world"));
    ASSERT_TRUE(
        (*created)->appendLine(makeLine(2U, DetectedLogLevel::Info, "日本語トークン"), "日本語トークン"));
    ASSERT_TRUE((*created)->finalize(2U));

    const auto parsed = parseFilterQuery(R"(contains(message, "日本語"))");
    ASSERT_TRUE(parsed);

    const auto plan = planQueryPushdown(*parsed);
    EXPECT_FALSE(plan.has_value());

    const auto opened = SqliteIndexStore::open(databasePath);
    ASSERT_TRUE(opened);

    const auto allLines = (*opened)->fetchAllLines();
    ASSERT_TRUE(allLines);

    const QueryEvaluator evaluator(*parsed);
    std::size_t matchCount = 0U;

    for (const IndexedLine& line : *allLines)
    {
        if (evaluator.matches(line))
        {
            ++matchCount;
        }
    }

    EXPECT_EQ(1U, matchCount);

    cleanupWorkspace(workspace);
}

TEST(SqliteIndexStoreFtsTest, CombinedCjkContainsUsesEvaluatorFallback)
{
    const Path workspace = testWorkspace();
    const Path sourcePath = writeTempSource(workspace, "fts source\n");
    const Path databasePath = workspace.append("index.db");
    const IndexMetadata metadata = makeMetadata(sourcePath);

    const auto created = SqliteIndexStore::create(databasePath, metadata);
    ASSERT_TRUE(created);

    ASSERT_TRUE((*created)->appendLine(makeLine(1U, DetectedLogLevel::Error, "日本語トークン"), "日本語トークン"));
    ASSERT_TRUE((*created)->appendLine(makeLine(2U, DetectedLogLevel::Info, "日本語トークン"), "日本語トークン"));
    ASSERT_TRUE((*created)->finalize(2U));

    const auto parsed = parseFilterQuery(R"(level == ERROR AND contains(message, "日本語"))");
    ASSERT_TRUE(parsed);

    const auto plan = planQueryPushdown(*parsed);
    EXPECT_FALSE(plan.has_value());

    const auto allLines = (*created)->fetchAllLines();
    ASSERT_TRUE(allLines);

    const QueryEvaluator evaluator(*parsed);
    std::size_t matchCount = 0U;

    for (const IndexedLine& line : *allLines)
    {
        if (evaluator.matches(line))
        {
            ++matchCount;
        }
    }

    EXPECT_EQ(1U, matchCount);

    cleanupWorkspace(workspace);
}
