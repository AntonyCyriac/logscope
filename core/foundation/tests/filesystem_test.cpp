/**
 * @file filesystem_test.cpp
 * @brief Unit tests for FileSystem.
 */

#include <fstream>

#include <gtest/gtest.h>

#include "foundation.hpp"

using scope::foundation::FileSystem;
using scope::foundation::Path;

namespace
{

class FileSystemTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        m_testFile = Path("filesystem_test_temp.log");

        std::ofstream stream(m_testFile.string());

        stream << "test";
    }

    void TearDown() override
    {
        std::remove(m_testFile.string().c_str());
    }

    Path m_testFile;
};

} // namespace

TEST_F(FileSystemTest, Exists)
{
    auto result = FileSystem::exists(m_testFile);

    ASSERT_TRUE(result.hasValue());
    EXPECT_TRUE(*result);
}

TEST_F(FileSystemTest, IsFile)
{
    auto result = FileSystem::isFile(m_testFile);

    ASSERT_TRUE(result.hasValue());
    EXPECT_TRUE(*result);
}

TEST_F(FileSystemTest, IsNotDirectory)
{
    auto result = FileSystem::isDirectory(m_testFile);

    ASSERT_TRUE(result.hasValue());
    EXPECT_FALSE(*result);
}

TEST_F(FileSystemTest, FileSize)
{
    auto result = FileSystem::fileSize(m_testFile);

    ASSERT_TRUE(result.hasValue());
    EXPECT_EQ(4U, *result);
}

TEST_F(FileSystemTest, MissingPath)
{
    auto result = FileSystem::exists(Path("nonexistent_file_12345.log"));

    ASSERT_TRUE(result.hasValue());
    EXPECT_FALSE(*result);
}
