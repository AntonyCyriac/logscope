/**
 * @file sqlite_index_store_incremental_append_test.cpp
 */

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include <sqlite3.h>

#include "analysis.hpp"
#include "source.hpp"
#include "index_store_factory.hpp"
#include "sqlite_index_store.hpp"

using scope::analysis::AnalysisConfig;
using scope::analysis::AnalysisEngine;
using scope::analysis::DetectedLogLevel;
using scope::analysis::IndexedLine;
using scope::analysis::LogFormat;
using scope::foundation::Path;
using scope::source::SourceManager;
using scope::storage::IndexFingerprint;
using scope::storage::StorageConfig;
using scope::storage::createIndexStore;

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

IndexedLine makeLine(const std::uint64_t lineNumber, const std::string& content)
{
    IndexedLine line;
    line.lineNumber = lineNumber;
    line.level = DetectedLogLevel::Info;
    line.messageExcerpt = content;
    line.contentExcerpt = content;

    return line;
}

} // namespace

TEST(SqliteIndexStoreIncrementalAppendTest, AppendsNewLinesThroughAnalysisEngine)
{
    const Path workspace = testWorkspace();
    Path sourcePath = writeSource(workspace, "alpha\nbeta\n");
    const Path indexPath = workspace.append("index.db");

    StorageConfig storageConfig = StorageConfig::defaults();
    storageConfig.persistIndex = true;
    storageConfig.reuseIndex = true;
    storageConfig.indexPath = indexPath;

    AnalysisConfig analysisConfig = AnalysisConfig::defaults();
    analysisConfig.storage = storageConfig;

    SourceManager sourceManager;
    AnalysisEngine engine;

    {
        auto dataset = sourceManager.open(sourcePath);
        ASSERT_TRUE(dataset);
        const auto model = engine.analyze(*dataset, analysisConfig);
        ASSERT_TRUE(model);
        EXPECT_EQ(2U, model->totalLines());
        ASSERT_NE(nullptr, model->indexStore());
        EXPECT_EQ(2U, model->indexStore()->storedLineCount());
    }

    {
        std::ofstream append(sourcePath.string(), std::ios::app);
        append << "gamma\n";
    }

    {
        auto dataset = sourceManager.open(sourcePath);
        ASSERT_TRUE(dataset);
        const auto model = engine.analyze(*dataset, analysisConfig);
        ASSERT_TRUE(model);
        EXPECT_EQ(3U, model->totalLines());
        ASSERT_NE(nullptr, model->indexStore());
        EXPECT_EQ(3U, model->indexStore()->storedLineCount());

        const auto lines = model->indexStore()->fetchAllLines();
        ASSERT_TRUE(lines);
        ASSERT_EQ(3U, lines->size());
        EXPECT_EQ(3U, lines->back().lineNumber);
    }

    cleanupWorkspace(workspace);
}

TEST(SqliteIndexStoreIncrementalAppendTest, UpdatesFingerprintOnFinalize)
{
    const Path workspace = testWorkspace();
    Path sourcePath = writeSource(workspace, "alpha\n");
    const Path indexPath = workspace.append("index.db");

    StorageConfig storageConfig = StorageConfig::defaults();
    storageConfig.persistIndex = true;
    storageConfig.indexPath = indexPath;

    const auto initialFingerprint = IndexFingerprint::compute(sourcePath);
    ASSERT_TRUE(initialFingerprint);

    {
        const auto created = createIndexStore(storageConfig, *initialFingerprint, sourcePath, LogFormat::PlainText);
        ASSERT_TRUE(created);
        ASSERT_TRUE((*created)->appendLine(makeLine(1U, "alpha\n"), "alpha\n"));
        ASSERT_TRUE((*created)->finalize(1U));
    }

    {
        std::ofstream append(sourcePath.string(), std::ios::app);
        append << "beta\n";
        append.flush();
    }

    const auto grownFingerprint = IndexFingerprint::compute(sourcePath);
    ASSERT_TRUE(grownFingerprint);
    EXPECT_NE(initialFingerprint->value(), grownFingerprint->value());

    storageConfig.reuseIndex = true;

    SourceManager sourceManager;
    AnalysisConfig analysisConfig = AnalysisConfig::defaults();
    analysisConfig.storage = storageConfig;

    auto dataset = sourceManager.open(sourcePath);
    ASSERT_TRUE(dataset);
    const auto model = AnalysisEngine{}.analyze(*dataset, analysisConfig);
    ASSERT_TRUE(model);
    EXPECT_EQ(2U, model->indexStore()->storedLineCount());
    EXPECT_EQ(grownFingerprint->value(), model->indexStore()->metadata().fingerprint);

    cleanupWorkspace(workspace);
}
