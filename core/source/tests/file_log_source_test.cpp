/**
 * @file file_log_source_test.cpp
 * @brief Unit tests for FileLogSource.
 */

#include <fstream>

#include <gtest/gtest.h>

#include "source.hpp"

using scope::foundation::Path;
using scope::source::FileLogSource;
using scope::source::LogSource;

namespace
{

class FileLogSourceTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        m_testFile = Path("file_log_source_test.log");

        std::ofstream stream(m_testFile.string());

        stream << "alpha\n";
        stream << "beta\n";
    }

    void TearDown() override
    {
        std::remove(m_testFile.string().c_str());
    }

    Path m_testFile;
};

} // namespace

TEST_F(FileLogSourceTest, OpenAndReadLines)
{
    const auto sourceResult = FileLogSource::open(m_testFile);

    ASSERT_TRUE(sourceResult.hasValue());

    LogSource& logSource = **sourceResult;

    EXPECT_EQ(m_testFile.string(), logSource.path().string());

    std::string line;

    const auto firstLine = logSource.readLine(line);

    ASSERT_TRUE(firstLine.hasValue());
    EXPECT_TRUE(*firstLine);
    EXPECT_EQ("alpha", line);

    const auto secondLine = logSource.readLine(line);

    ASSERT_TRUE(secondLine.hasValue());
    EXPECT_TRUE(*secondLine);
    EXPECT_EQ("beta", line);

    const auto endOfFile = logSource.readLine(line);

    ASSERT_TRUE(endOfFile.hasValue());
    EXPECT_FALSE(*endOfFile);
}

TEST_F(FileLogSourceTest, OpenMissingFile)
{
    const auto sourceResult = FileLogSource::open(Path("missing_file.log"));

    ASSERT_TRUE(sourceResult.hasError());
}
