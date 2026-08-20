/**
 * @file source_discovery_test.cpp
 * @brief Ingestion integrity discovery tests (ADR-013 / II.1, II.3, II.7).
 */

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

#include "discovery_census.hpp"
#include "gtest_temp_path.hpp"
#include "source_discovery.hpp"
#include "source_manager.hpp"

using scope::foundation::ErrorCode;
using scope::foundation::Path;
using scope::source::CandidateDisposition;
using scope::source::DiscoveryOptions;
using scope::source::SkipReason;
using scope::source::SourceManager;
using scope::source::discoverSource;
using scope::source::isArchivePath;

namespace
{

void writeTextFile(const Path& path, const std::string& content)
{
    const std::filesystem::path parent = std::filesystem::path(path.string()).parent_path();

    if (!parent.empty())
    {
        std::filesystem::create_directories(parent);
    }

    std::ofstream stream(path.string());
    stream << content;
}

void writeBinaryFile(const Path& path)
{
    std::ofstream stream(path.string(), std::ios::binary);

    for (int index = 0; index < 256; ++index)
    {
        stream.put(static_cast<char>(index % 32));
    }
}

} // namespace

TEST(SourceDiscoveryTest, NestedBundleIngestsLeafLogs)
{
    const Path root(logscope::gtest::uniqueTestPath("_bundle_root"));
    const Path leaf = root.append("subsys").append("nested").append("app.log");

    writeTextFile(leaf, "nested line one\nnested line two\n");

    const auto result = discoverSource(root, DiscoveryOptions{});

    ASSERT_TRUE(result.hasValue());
    EXPECT_GE(result->census.candidatesFound, 1U);
    EXPECT_EQ(1U, result->census.analyzedCount);
    ASSERT_FALSE(result->ingestStreams.empty());

    bool foundLeaf = false;

    for (const scope::source::DiscoveryEntry& entry : result->census.entries)
    {
        if (entry.relativePath.find("app.log") != std::string::npos &&
            entry.disposition == CandidateDisposition::Analyzed)
        {
            foundLeaf = true;
        }
    }

    EXPECT_TRUE(foundLeaf);

    std::filesystem::remove_all(root.string());
}

TEST(SourceDiscoveryTest, AllSkippedIndeterminate)
{
    const Path directory(logscope::gtest::uniqueTestPath("_binary_dir"));

    std::filesystem::create_directories(directory.string());

    writeBinaryFile(directory.append("one.bin"));
    writeBinaryFile(directory.append("two.dat"));

    SourceManager manager;

    const auto validateResult = manager.validate(directory);

    ASSERT_TRUE(validateResult.hasError());
    EXPECT_EQ(ErrorCode::Indeterminate, validateResult.error().code());

    const auto openResult = manager.open(directory);

    ASSERT_TRUE(openResult.hasError());
    EXPECT_EQ(ErrorCode::Indeterminate, openResult.error().code());

    const auto discovery = discoverSource(directory, DiscoveryOptions{});

    ASSERT_TRUE(discovery.hasValue());
    EXPECT_GT(discovery->census.candidatesFound, 0U);
    EXPECT_EQ(0U, discovery->census.analyzedCount);

    for (const scope::source::DiscoveryEntry& entry : discovery->census.entries)
    {
        EXPECT_EQ(CandidateDisposition::Skipped, entry.disposition);
        ASSERT_TRUE(entry.skipReason.has_value());
        EXPECT_EQ(SkipReason::BinaryContent, *entry.skipReason);
    }

    std::filesystem::remove_all(directory.string());
}

TEST(SourceDiscoveryTest, SingleFileRootUsesAbsolutePath)
{
    const Path file(logscope::gtest::uniqueTestPath("_single.log"));

    writeTextFile(file, "one line\n");

    const auto result = discoverSource(file, DiscoveryOptions{});

    ASSERT_TRUE(result.hasValue());
    ASSERT_EQ(1U, result->ingestStreams.size());
    ASSERT_EQ(1U, result->ingestStreams.front().orderedFiles.size());
    EXPECT_EQ(file.string(), result->ingestStreams.front().orderedFiles.front().absolutePath.string());

    std::remove(file.string().c_str());
}

TEST(SourceDiscoveryTest, ArchivePathRejected)
{
    const Path archivePath(logscope::gtest::uniqueTestPath(".tar.gz"));

    {
        std::ofstream stream(archivePath.string(), std::ios::binary);
        stream << "not a real archive";
    }

    EXPECT_TRUE(isArchivePath(archivePath));

    SourceManager manager;

    const auto validateResult = manager.validate(archivePath);

    ASSERT_TRUE(validateResult.hasError());
    EXPECT_EQ(ErrorCode::InvalidArgument, validateResult.error().code());
    EXPECT_NE(validateResult.error().message().find("Extract"), std::string::npos);

    std::remove(archivePath.string().c_str());
}
