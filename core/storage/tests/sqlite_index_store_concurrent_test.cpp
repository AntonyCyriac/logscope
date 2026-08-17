/**
 * @file sqlite_index_store_concurrent_test.cpp
 * @brief Regression tests for concurrent index creation (#163).
 */

#include <atomic>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "gtest_temp_path.hpp"
#include "index_build_lock.hpp"
#include "sqlite_index_store.hpp"

using scope::analysis::DetectedLogLevel;
using scope::analysis::IndexedLine;
using scope::analysis::LogFormat;
using scope::foundation::Path;
using scope::storage::IndexMetadata;
using scope::storage::SqliteIndexStore;

namespace
{

IndexedLine makeLine(const std::uint64_t lineNumber, const std::string& message)
{
    IndexedLine line;
    line.lineNumber = lineNumber;
    line.level = DetectedLogLevel::Info;
    line.messageExcerpt = message;
    line.contentExcerpt = message;

    return line;
}

Path writeTempSource(const std::string& suffix, const std::string& content)
{
    const Path sourcePath(logscope::gtest::uniqueTestPath("_" + suffix + ".log"));
    std::ofstream output(sourcePath.string());
    output << content;
    output.close();

    return sourcePath;
}

void cleanupDatabaseArtifacts(const Path& databasePath)
{
    scope::storage::removeDatabaseArtifacts(databasePath);
    scope::storage::removeDatabaseArtifacts(scope::storage::indexBuildingDatabasePath(databasePath));
    std::error_code error;
    std::filesystem::remove(scope::storage::indexBuildLockPath(databasePath).string(), error);
}

} // namespace

TEST(SqliteIndexStoreConcurrentTest, ConcurrentCreateSamePathAvoidsDiskIoError)
{
    const Path databasePath(logscope::gtest::uniqueTestPath("_concurrent_create.db"));
    cleanupDatabaseArtifacts(databasePath);

    const Path sourcePath = writeTempSource("concurrent_create", "info line one\ninfo line two\n");

    IndexMetadata metadata;
    metadata.fingerprint = "concurrent-fingerprint";
    metadata.sourcePath = sourcePath;
    metadata.format = LogFormat::PlainText;

    constexpr int kThreads = 4;
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};
    std::atomic<int> diskIoErrors{0};
    std::atomic<int> lockMessages{0};

    for (int threadIndex = 0; threadIndex < kThreads; ++threadIndex)
    {
        threads.emplace_back([&, threadIndex]()
                             {
                                 const auto created = SqliteIndexStore::create(databasePath, metadata);

                                 if (!created)
                                 {
                                     const std::string message = created.error().message();

                                     if (message.find("disk I/O error") != std::string::npos)
                                     {
                                         ++diskIoErrors;
                                     }

                                     if (message.find("Index is locked by another process") != std::string::npos)
                                     {
                                         ++lockMessages;
                                     }

                                     return;
                                 }

                                 const std::string lineText = "thread-" + std::to_string(threadIndex);
                                 ASSERT_TRUE((*created)->appendLine(makeLine(threadIndex + 1U, lineText), lineText));

                                 if ((*created)->finalize(threadIndex + 1U))
                                 {
                                     ++successCount;
                                 }
                             });
    }

    for (std::thread& thread : threads)
    {
        thread.join();
    }

    EXPECT_EQ(0, diskIoErrors.load());
    EXPECT_GE(successCount.load(), 1);
    EXPECT_TRUE(std::filesystem::exists(databasePath.string()));
    EXPECT_FALSE(std::filesystem::exists(
        scope::storage::indexBuildingDatabasePath(databasePath).string()));

    cleanupDatabaseArtifacts(databasePath);
}

TEST(SqliteIndexStoreConcurrentTest, ConcurrentOpenExistingIndexSucceeds)
{
    const Path databasePath(logscope::gtest::uniqueTestPath("_concurrent_open.db"));
    cleanupDatabaseArtifacts(databasePath);

    const Path sourcePath = writeTempSource("concurrent_open", "warn line\n");

    IndexMetadata metadata;
    metadata.fingerprint = "concurrent-open-fingerprint";
    metadata.sourcePath = sourcePath;
    metadata.format = LogFormat::PlainText;

    const auto created = SqliteIndexStore::create(databasePath, metadata);
    ASSERT_TRUE(created);
    ASSERT_TRUE((*created)->appendLine(makeLine(1U, "warn line"), "warn line"));
    ASSERT_TRUE((*created)->finalize(1U));

    constexpr int kThreads = 4;
    std::vector<std::thread> threads;
    std::atomic<int> openSuccess{0};

    for (int threadIndex = 0; threadIndex < kThreads; ++threadIndex)
    {
        threads.emplace_back([&]()
                             {
                                 const auto opened = SqliteIndexStore::open(databasePath);

                                 if (opened)
                                 {
                                     ++openSuccess;
                                 }
                             });
    }

    for (std::thread& thread : threads)
    {
        thread.join();
    }

    EXPECT_EQ(kThreads, openSuccess.load());

    cleanupDatabaseArtifacts(databasePath);
}
