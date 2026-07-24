/**
 * @file sqlite_index_store_json_fields_test.cpp
 */

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <optional>
#include <sqlite3.h>

#include "index_fingerprint.hpp"
#include "index_store_options.hpp"
#include "query_parser.hpp"
#include "query_planner.hpp"
#include "sqlite_index_store.hpp"

using scope::analysis::DetectedLogLevel;
using scope::analysis::IndexedLine;
using scope::analysis::LogFormat;
using scope::foundation::Path;
using scope::query::parseFilterQuery;
using scope::storage::IndexFingerprint;
using scope::storage::IndexMetadata;
using scope::storage::IndexStoreOptions;
using scope::storage::SqliteIndexStore;
using scope::storage::planQueryPushdown;

namespace
{

IndexedLine makeJsonLine(const std::uint64_t lineNumber, const DetectedLogLevel level,
                         std::vector<std::pair<std::string, std::string>> jsonFieldValues)
{
    IndexedLine line;
    line.lineNumber = lineNumber;
    line.level = level;
    line.messageExcerpt = "sample";
    line.contentExcerpt = "sample";
    line.topLevelKeys = {"service", "level"};
    line.jsonFieldValues = std::move(jsonFieldValues);

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

std::optional<std::size_t> countJsonFieldRows(const Path& databasePath)
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

    if (sqlite3_prepare_v2(database, "SELECT COUNT(*) FROM line_json_fields;", -1, &statement, nullptr) != SQLITE_OK)
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
    metadata.format = LogFormat::JsonLines;

    return metadata;
}

} // namespace

TEST(SqliteIndexStoreJsonFieldsTest, PersistsTopLevelJsonFieldValues)
{
    const Path workspace = testWorkspace();
    const Path databasePath = workspace.append("index.db");
    const Path sourcePath = writeTempSource(workspace, "{\"service\":\"PCF\"}\n");

    const auto metadata = makeMetadata(sourcePath);
    const auto created = SqliteIndexStore::create(databasePath, metadata);
    ASSERT_TRUE(created);

    const auto line = makeJsonLine(1U, DetectedLogLevel::Info, {{"service", "PCF"}, {"level", "info"}});
    ASSERT_TRUE((*created)->appendLine(line, R"({"service":"PCF","level":"info"})"));
    ASSERT_TRUE((*created)->finalize(1U));

    const auto rowCount = countJsonFieldRows(databasePath);
    ASSERT_TRUE(rowCount.has_value());
    EXPECT_EQ(2U, *rowCount);

    cleanupWorkspace(workspace);
}

TEST(SqliteIndexStoreJsonFieldsTest, PlainTextLineStoresNoJsonFieldRows)
{
    const Path workspace = testWorkspace();
    const Path databasePath = workspace.append("index.db");
    const Path sourcePath = writeTempSource(workspace, "plain line\n");

    IndexMetadata metadata;
    metadata.fingerprint = IndexFingerprint::compute(sourcePath)->value();
    metadata.sourcePath = sourcePath;
    metadata.format = LogFormat::PlainText;

    const auto created = SqliteIndexStore::create(databasePath, metadata);
    ASSERT_TRUE(created);

    IndexedLine line;
    line.lineNumber = 1U;
    line.level = DetectedLogLevel::Info;
    line.messageExcerpt = "plain line";
    line.contentExcerpt = "plain line";

    ASSERT_TRUE((*created)->appendLine(line, "plain line"));
    ASSERT_TRUE((*created)->finalize(1U));

    const auto rowCount = countJsonFieldRows(databasePath);
    ASSERT_TRUE(rowCount.has_value());
    EXPECT_EQ(0U, *rowCount);

    cleanupWorkspace(workspace);
}

TEST(SqliteIndexStoreJsonFieldsTest, FetchesLinesMatchingJsonFieldPushdown)
{
    const Path workspace = testWorkspace();
    const Path databasePath = workspace.append("index.db");
    const Path sourcePath = writeTempSource(workspace, "{\"service\":\"PCF\"}\n{\"service\":\"OTHER\"}\n");

    const auto metadata = makeMetadata(sourcePath);
    const auto created = SqliteIndexStore::create(databasePath, metadata);
    ASSERT_TRUE(created);

    ASSERT_TRUE((*created)->appendLine(makeJsonLine(1U, DetectedLogLevel::Error, {{"service", "PCF"}}),
                                       R"({"service":"PCF"})"));
    ASSERT_TRUE((*created)->appendLine(makeJsonLine(2U, DetectedLogLevel::Info, {{"service", "OTHER"}}),
                                       R"({"service":"OTHER"})"));
    ASSERT_TRUE((*created)->finalize(2U));

    const auto parsed = parseFilterQuery(R"(service == "PCF")");
    ASSERT_TRUE(parsed);

    const auto plan = planQueryPushdown(*parsed);
    ASSERT_TRUE(plan);

    const auto opened = SqliteIndexStore::open(databasePath);
    ASSERT_TRUE(opened);

    const auto lines = (*opened)->fetchLinesWhere(plan->sqlWhere);
    ASSERT_TRUE(lines);
    ASSERT_EQ(1U, lines->size());
    EXPECT_EQ(1U, lines->front().lineNumber);
    EXPECT_EQ(DetectedLogLevel::Error, lines->front().level);
    ASSERT_EQ(1U, lines->front().jsonFieldValues.size());
    EXPECT_EQ("service", lines->front().jsonFieldValues.front().first);
    EXPECT_EQ("PCF", lines->front().jsonFieldValues.front().second);

    cleanupWorkspace(workspace);
}

TEST(SqliteIndexStoreJsonFieldsTest, FetchesLinesMatchingCombinedJsonFieldAndLevel)
{
    const Path workspace = testWorkspace();
    const Path databasePath = workspace.append("index.db");
    const Path sourcePath = writeTempSource(workspace, "jsonl\n");

    const auto metadata = makeMetadata(sourcePath);
    const auto created = SqliteIndexStore::create(databasePath, metadata);
    ASSERT_TRUE(created);

    ASSERT_TRUE((*created)->appendLine(makeJsonLine(1U, DetectedLogLevel::Error, {{"service", "PCF"}}),
                                       R"({"service":"PCF"})"));
    ASSERT_TRUE((*created)->appendLine(makeJsonLine(2U, DetectedLogLevel::Info, {{"service", "PCF"}}),
                                       R"({"service":"PCF"})"));
    ASSERT_TRUE((*created)->finalize(2U));

    const auto parsed = parseFilterQuery(R"(service == "PCF" AND level == ERROR)");
    ASSERT_TRUE(parsed);

    const auto plan = planQueryPushdown(*parsed);
    ASSERT_TRUE(plan);

    const auto lines = (*created)->fetchLinesWhere(plan->sqlWhere);
    ASSERT_TRUE(lines);
    ASSERT_EQ(1U, lines->size());
    EXPECT_EQ(1U, lines->front().lineNumber);

    cleanupWorkspace(workspace);
}
