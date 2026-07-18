/**
 * @file source_manager_test.cpp
 * @brief Unit tests for SourceManager.
 */

#include <fstream>

#include <filesystem>
#include <gtest/gtest.h>

#include "source.hpp"

using scope::foundation::ErrorCode;
using scope::foundation::Path;
using scope::source::LogSource;
using scope::source::SourceManager;

namespace
{

class SourceManagerTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        m_testFile = Path("source_manager_test.log");

        std::ofstream stream(m_testFile.string());

        stream << "line one\n";
        stream << "line two\n";
        stream << "line three\n";
    }

    void TearDown() override
    {
        std::remove(m_testFile.string().c_str());
    }

    Path m_testFile;
};

} // namespace

TEST_F(SourceManagerTest, ValidateExistingFile)
{
    SourceManager manager;

    const auto result = manager.validate(m_testFile);

    ASSERT_TRUE(result.hasValue());
    EXPECT_TRUE(*result);
}

TEST_F(SourceManagerTest, ValidateMissingFile)
{
    SourceManager manager;

    const auto result = manager.validate(Path("missing_source.log"));

    ASSERT_TRUE(result.hasError());
    EXPECT_EQ(ErrorCode::FileNotFound, result.error().code());
}

TEST_F(SourceManagerTest, OpenReadsAllLines)
{
    SourceManager manager;

    auto datasetResult = manager.open(m_testFile);

    ASSERT_TRUE(datasetResult.hasValue());
    EXPECT_TRUE(datasetResult->isValid());
    EXPECT_EQ(m_testFile.string(), datasetResult->path().string());

    std::string line;
    std::size_t lineCount = 0;
    LogSource& logSource = datasetResult->source();

    while (true)
    {
        const auto readResult = logSource.readLine(line);

        ASSERT_TRUE(readResult.hasValue());

        if (!*readResult)
        {
            break;
        }

        ++lineCount;
    }

    EXPECT_EQ(3U, lineCount);
}

TEST_F(SourceManagerTest, OpenMissingFile)
{
    SourceManager manager;

    const auto result = manager.open(Path("missing_source.log"));

    ASSERT_TRUE(result.hasError());
    EXPECT_EQ(ErrorCode::FileNotFound, result.error().code());
}

TEST_F(SourceManagerTest, ValidateStdinPath)
{
    SourceManager manager;

    const auto result = manager.validate(Path("-"));

    ASSERT_TRUE(result.hasValue());
    EXPECT_TRUE(*result);
}

TEST_F(SourceManagerTest, ValidateEmptyDirectory)
{
    const Path directoryPath("source_manager_empty_dir");

    std::filesystem::create_directory(directoryPath.string());

    SourceManager manager;

    const auto result = manager.validate(directoryPath);

    ASSERT_TRUE(result.hasError());
    EXPECT_EQ(ErrorCode::InvalidArgument, result.error().code());

    std::filesystem::remove(directoryPath.string());
}

TEST_F(SourceManagerTest, OpenDirectoryReadsAllLogFiles)
{
    const Path directoryPath("source_manager_dir");

    std::filesystem::create_directory(directoryPath.string());

    const Path firstFile = directoryPath.append("first.log");
    const Path secondFile = directoryPath.append("second.log");

    {
        std::ofstream stream(firstFile.string());
        stream << "alpha\n";
        stream << "beta\n";
    }

    {
        std::ofstream stream(secondFile.string());
        stream << "gamma\n";
    }

    SourceManager manager;

    {
        auto datasetResult = manager.open(directoryPath);

        ASSERT_TRUE(datasetResult.hasValue());
        EXPECT_EQ(directoryPath.string(), datasetResult->path().string());

        std::string line;
        std::size_t lineCount = 0;
        LogSource& logSource = datasetResult->source();

        while (true)
        {
            const auto readResult = logSource.readLine(line);

            ASSERT_TRUE(readResult.hasValue());

            if (!*readResult)
            {
                break;
            }

            ++lineCount;
        }

        EXPECT_EQ(3U, lineCount);
    }

    std::remove(firstFile.string().c_str());
    std::remove(secondFile.string().c_str());
    std::filesystem::remove(directoryPath.string());
}
