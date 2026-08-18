/**
 * @file index_reuse_test.cpp
 */

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include <sqlite3.h>

#include "index_fingerprint.hpp"
#include "index_reuse.hpp"
#include "index_store_factory.hpp"
#include "schema_version.hpp"
#include "sqlite_index_store.hpp"

using scope::analysis::DetectedLogLevel;
using scope::analysis::IndexedLine;
using scope::analysis::LogFormat;
using scope::foundation::Path;
using scope::storage::IndexFingerprint;
using scope::storage::IndexMetadata;
using scope::storage::IndexReuseMode;
using scope::storage::SqliteIndexStore;
using scope::storage::StorageConfig;
using scope::storage::createIndexStore;
using scope::storage::isUnsupportedSchemaVersion;
using scope::storage::prepareIndexReuse;
using scope::storage::resolveIndexPath;

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

StorageConfig makeReuseConfig(const Path& workspace)
{
    StorageConfig config = StorageConfig::defaults();
    config.persistIndex = true;
    config.reuseIndex = true;
    config.indexDirectory = workspace;

    return config;
}

} // namespace

TEST(IndexReuseTest, ReusesUnchangedSource)
{
    const Path workspace = testWorkspace();
    const Path sourcePath = writeSource(workspace, "alpha\nbeta\n");
    StorageConfig config = makeReuseConfig(workspace);

    const auto fingerprint = IndexFingerprint::compute(sourcePath);
    ASSERT_TRUE(fingerprint);

    {
        const auto created = createIndexStore(config, *fingerprint, sourcePath, LogFormat::PlainText);
        ASSERT_TRUE(created);
        ASSERT_TRUE((*created)->appendLine(makeLine(1U), "alpha\n"));
        ASSERT_TRUE((*created)->appendLine(makeLine(2U), "beta\n"));
        ASSERT_TRUE((*created)->finalize(2U));
    }

    const auto prepared = prepareIndexReuse(config, *fingerprint, sourcePath);
    ASSERT_TRUE(prepared);
    EXPECT_EQ(IndexReuseMode::Unchanged, prepared->mode);
    ASSERT_NE(nullptr, prepared->store);
    EXPECT_EQ(2U, prepared->store->metadata().totalLines);

    cleanupWorkspace(workspace);
}

TEST(IndexReuseTest, PreparesAppendWhenSourceGrows)
{
    const Path workspace = testWorkspace();
    Path sourcePath = writeSource(workspace, "alpha\nbeta\n");
    StorageConfig config = makeReuseConfig(workspace);

    const auto fingerprint = IndexFingerprint::compute(sourcePath);
    ASSERT_TRUE(fingerprint);

    {
        const auto created = createIndexStore(config, *fingerprint, sourcePath, LogFormat::PlainText);
        ASSERT_TRUE(created);
        ASSERT_TRUE((*created)->appendLine(makeLine(1U), "alpha\n"));
        ASSERT_TRUE((*created)->appendLine(makeLine(2U), "beta\n"));
        ASSERT_TRUE((*created)->finalize(2U));
    }

    {
        std::ofstream append(sourcePath.string(), std::ios::app);
        append << "gamma\n";
    }

    const auto grownFingerprint = IndexFingerprint::compute(sourcePath);
    ASSERT_TRUE(grownFingerprint);

    const auto prepared = prepareIndexReuse(config, *grownFingerprint, sourcePath);
    ASSERT_TRUE(prepared);
    EXPECT_EQ(IndexReuseMode::Append, prepared->mode);
    ASSERT_NE(nullptr, prepared->store);
    EXPECT_EQ(2U, prepared->linesToSkip);

    cleanupWorkspace(workspace);
}

TEST(IndexReuseTest, DeletesIndexOnTruncateAndRequestsRebuild)
{
    const Path workspace = testWorkspace();
    Path sourcePath = writeSource(workspace, "alpha\nbeta\n");
    StorageConfig config = makeReuseConfig(workspace);

    const auto fingerprint = IndexFingerprint::compute(sourcePath);
    ASSERT_TRUE(fingerprint);

    const Path databasePath = resolveIndexPath(config, sourcePath, *fingerprint);

    {
        const auto created = createIndexStore(config, *fingerprint, sourcePath, LogFormat::PlainText);
        ASSERT_TRUE(created);
        ASSERT_TRUE((*created)->appendLine(makeLine(1U), "alpha\n"));
        ASSERT_TRUE((*created)->appendLine(makeLine(2U), "beta\n"));
        ASSERT_TRUE((*created)->finalize(2U));
    }

    writeSourceAt(sourcePath, "x\n");

    const auto truncatedFingerprint = IndexFingerprint::compute(sourcePath);
    ASSERT_TRUE(truncatedFingerprint);

    const auto prepared = prepareIndexReuse(config, *truncatedFingerprint, sourcePath);
    ASSERT_TRUE(prepared);
    EXPECT_EQ(IndexReuseMode::Rebuild, prepared->mode);
    EXPECT_EQ(nullptr, prepared->store);

    const auto exists = std::filesystem::exists(databasePath.string());
    EXPECT_FALSE(exists);

    cleanupWorkspace(workspace);
}

TEST(IndexReuseTest, RebuildsWhenIncrementalAppendDisabled)
{
    const Path workspace = testWorkspace();
    Path sourcePath = writeSource(workspace, "alpha\n");
    StorageConfig config = makeReuseConfig(workspace);
    config.incrementalAppend = false;
    config.indexPath = workspace.append("fixed.db");

    const auto fingerprint = IndexFingerprint::compute(sourcePath);
    ASSERT_TRUE(fingerprint);

    {
        const auto created = createIndexStore(config, *fingerprint, sourcePath, LogFormat::PlainText);
        ASSERT_TRUE(created);
        ASSERT_TRUE((*created)->appendLine(makeLine(1U), "alpha\n"));
        ASSERT_TRUE((*created)->finalize(1U));
    }

    {
        std::ofstream append(sourcePath.string(), std::ios::app);
        append << "beta\n";
    }

    const auto grownFingerprint = IndexFingerprint::compute(sourcePath);
    ASSERT_TRUE(grownFingerprint);

    const auto prepared = prepareIndexReuse(config, *grownFingerprint, sourcePath);
    ASSERT_TRUE(prepared);
    EXPECT_EQ(IndexReuseMode::Rebuild, prepared->mode);

    cleanupWorkspace(workspace);
}

TEST(IndexReuseTest, UsesStablePathKeyForDefaultIndexLocation)
{
    const Path workspace = testWorkspace();
    const Path sourcePath = writeSource(workspace, "alpha\n");
    StorageConfig config = makeReuseConfig(workspace);

    const auto fingerprint = IndexFingerprint::compute(sourcePath);
    ASSERT_TRUE(fingerprint);

    const auto stableKey = IndexFingerprint::stablePathKey(sourcePath);
    ASSERT_TRUE(stableKey);

    const Path resolved = resolveIndexPath(config, sourcePath, *fingerprint);
    EXPECT_EQ(workspace.append(stableKey->value() + ".db").string(), resolved.string());

    cleanupWorkspace(workspace);
}

TEST(IndexReuseTest, RebuildsWhenSameSizeContentChanges)
{
    const Path workspace = testWorkspace();
    Path sourcePath = writeSource(workspace, "alpha\nbeta\n");
    StorageConfig config = makeReuseConfig(workspace);

    const auto fingerprint = IndexFingerprint::compute(sourcePath);
    ASSERT_TRUE(fingerprint);

    const Path databasePath = resolveIndexPath(config, sourcePath, *fingerprint);

    {
        const auto created = createIndexStore(config, *fingerprint, sourcePath, LogFormat::PlainText);
        ASSERT_TRUE(created);
        ASSERT_TRUE((*created)->appendLine(makeLine(1U), "alpha\n"));
        ASSERT_TRUE((*created)->appendLine(makeLine(2U), "beta\n"));
        ASSERT_TRUE((*created)->finalize(2U));
    }

    writeSourceAt(sourcePath, "alphX\nbetY\n");

    const auto rewrittenFingerprint = IndexFingerprint::compute(sourcePath);
    ASSERT_TRUE(rewrittenFingerprint);

    const auto prepared = prepareIndexReuse(config, *rewrittenFingerprint, sourcePath);
    ASSERT_TRUE(prepared);
    EXPECT_EQ(IndexReuseMode::Rebuild, prepared->mode);
    EXPECT_EQ(nullptr, prepared->store);
    EXPECT_FALSE(std::filesystem::exists(databasePath.string()));

    cleanupWorkspace(workspace);
}

TEST(IndexReuseTest, RebuildsWhenMidFileEditPrecedesGrowth)
{
    const Path workspace = testWorkspace();
    Path sourcePath = writeSource(workspace, "AAAA\nBBBB\n");
    StorageConfig config = makeReuseConfig(workspace);

    const auto fingerprint = IndexFingerprint::compute(sourcePath);
    ASSERT_TRUE(fingerprint);

    const Path databasePath = resolveIndexPath(config, sourcePath, *fingerprint);

    {
        const auto created = createIndexStore(config, *fingerprint, sourcePath, LogFormat::PlainText);
        ASSERT_TRUE(created);
        ASSERT_TRUE((*created)->appendLine(makeLine(1U), "AAAA\n"));
        ASSERT_TRUE((*created)->appendLine(makeLine(2U), "BBBB\n"));
        ASSERT_TRUE((*created)->finalize(2U));
    }

    writeSourceAt(sourcePath, "XXXX\nBBBB\nCCCC\n");

    const auto grownFingerprint = IndexFingerprint::compute(sourcePath);
    ASSERT_TRUE(grownFingerprint);

    const auto prepared = prepareIndexReuse(config, *grownFingerprint, sourcePath);
    ASSERT_TRUE(prepared);
    EXPECT_EQ(IndexReuseMode::Rebuild, prepared->mode);
    EXPECT_EQ(nullptr, prepared->store);
    EXPECT_FALSE(std::filesystem::exists(databasePath.string()));

    cleanupWorkspace(workspace);
}

TEST(IndexReuseTest, FailsClosedOnUnsupportedFutureSchema)
{
    const Path workspace = testWorkspace();
    const Path sourcePath = writeSource(workspace, "alpha\n");
    StorageConfig config = makeReuseConfig(workspace);

    const auto fingerprint = IndexFingerprint::compute(sourcePath);
    ASSERT_TRUE(fingerprint);

    const Path databasePath = resolveIndexPath(config, sourcePath, *fingerprint);

    {
        const auto created = createIndexStore(config, *fingerprint, sourcePath, LogFormat::PlainText);
        ASSERT_TRUE(created);
        ASSERT_TRUE((*created)->appendLine(makeLine(1U), "alpha\n"));
        ASSERT_TRUE((*created)->finalize(1U));
    }

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

    const auto prepared = prepareIndexReuse(config, *fingerprint, sourcePath);
    ASSERT_FALSE(prepared);
    EXPECT_TRUE(isUnsupportedSchemaVersion(prepared.error()));
    EXPECT_TRUE(std::filesystem::exists(databasePath.string()));

    cleanupWorkspace(workspace);
}
