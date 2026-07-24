/**
 * @file source_snapshot_test.cpp
 */

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include <sqlite3.h>

#include "source_snapshot.hpp"
#include "sqlite_index_store.hpp"

using scope::analysis::DetectedLogLevel;
using scope::analysis::IndexedLine;
using scope::analysis::LogFormat;
using scope::foundation::Path;
using scope::storage::IndexFingerprint;
using scope::storage::IndexMetadata;
using scope::storage::SourceChangeKind;
using scope::storage::SqliteIndexStore;
using scope::storage::compareSourceChange;
using scope::storage::readStoredSourceSnapshot;

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

Path writeSourceAt(const Path& sourcePath, const std::string& content)
{
    std::ofstream output(sourcePath.string(), std::ios::trunc);
    output << content;
    output.close();

    return sourcePath;
}

Path writeSource(const Path& workspace, const std::string& content)
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

IndexedLine makeLine(const std::uint64_t lineNumber)
{
    IndexedLine line;
    line.lineNumber = lineNumber;
    line.level = DetectedLogLevel::Info;
    line.messageExcerpt = "line";
    line.contentExcerpt = "line\n";

    return line;
}

void createIndexedStore(const Path& databasePath, const Path& sourcePath, const std::uint64_t lineCount)
{
    IndexMetadata metadata;
    metadata.fingerprint = IndexFingerprint::compute(sourcePath)->value();
    metadata.sourcePath = sourcePath;
    metadata.format = LogFormat::PlainText;

    const auto created = SqliteIndexStore::create(databasePath, metadata);
    ASSERT_TRUE(created);

    for (std::uint64_t lineNumber = 1U; lineNumber <= lineCount; ++lineNumber)
    {
        ASSERT_TRUE((*created)->appendLine(makeLine(lineNumber), "line\n"));
    }

    ASSERT_TRUE((*created)->finalize(lineCount));
}

} // namespace

TEST(SourceSnapshotTest, ComparesUnchangedSourceBySize)
{
    const Path workspace = testWorkspace();
    const Path sourcePath = writeSource(workspace, "alpha\nbeta\n");
    const Path databasePath = workspace.append("index.db");

    createIndexedStore(databasePath, sourcePath, 2U);

    sqlite3* database = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_open(databasePath.string().c_str(), &database));

    const auto snapshot = readStoredSourceSnapshot(database);
    ASSERT_TRUE(snapshot);

    const auto change = compareSourceChange(*snapshot, sourcePath);
    ASSERT_TRUE(change);
    EXPECT_EQ(SourceChangeKind::Unchanged, *change);

    sqlite3_close(database);
    cleanupWorkspace(workspace);
}

TEST(SourceSnapshotTest, DetectsGrowthAndTruncation)
{
    const Path workspace = testWorkspace();
    const Path sourcePath = writeSource(workspace, "alpha\n");
    const Path databasePath = workspace.append("index.db");

    createIndexedStore(databasePath, sourcePath, 1U);

    sqlite3* database = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_open(databasePath.string().c_str(), &database));

    const auto snapshot = readStoredSourceSnapshot(database);
    ASSERT_TRUE(snapshot);

    {
        std::ofstream append(sourcePath.string(), std::ios::app);
        append << "beta\n";
    }

    const auto grown = compareSourceChange(*snapshot, sourcePath);
    ASSERT_TRUE(grown);
    EXPECT_EQ(SourceChangeKind::Grown, *grown);

    writeSourceAt(sourcePath, "x\n");

    const auto truncated = compareSourceChange(*snapshot, sourcePath);
    ASSERT_TRUE(truncated);
    EXPECT_EQ(SourceChangeKind::Truncated, *truncated);

    sqlite3_close(database);
    cleanupWorkspace(workspace);
}

TEST(SourceSnapshotTest, IgnoresMtimeOnlyChange)
{
    const Path workspace = testWorkspace();
    const Path sourcePath = writeSource(workspace, "alpha\n");
    const Path databasePath = workspace.append("index.db");

    createIndexedStore(databasePath, sourcePath, 1U);

    sqlite3* database = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_open(databasePath.string().c_str(), &database));

    const auto snapshot = readStoredSourceSnapshot(database);
    ASSERT_TRUE(snapshot);

    const auto currentTime = std::filesystem::file_time_type::clock::now();
    std::filesystem::last_write_time(sourcePath.string(), currentTime);

    const auto change = compareSourceChange(*snapshot, sourcePath);
    ASSERT_TRUE(change);
    EXPECT_EQ(SourceChangeKind::Unchanged, *change);

    sqlite3_close(database);
    cleanupWorkspace(workspace);
}
