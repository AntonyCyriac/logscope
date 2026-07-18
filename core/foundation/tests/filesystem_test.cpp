/**
 * @file filesystem_test.cpp
 * @brief Unit tests for FileSystem.
 */

#include <fstream>

#include <filesystem>
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

TEST_F(FileSystemTest, ListRegularFiles)
{
    const Path directoryPath("filesystem_test_dir");

    std::filesystem::create_directory(directoryPath.string());

    const Path firstFile = directoryPath.append("b.log");
    const Path secondFile = directoryPath.append("a.log");

    {
        std::ofstream stream(firstFile.string());
        stream << "b";
    }

    {
        std::ofstream stream(secondFile.string());
        stream << "a";
    }

    const auto result = FileSystem::listRegularFiles(directoryPath);

    ASSERT_TRUE(result.hasValue());
    ASSERT_EQ(2U, result->size());
    EXPECT_EQ(secondFile.string(), (*result)[0].string());
    EXPECT_EQ(firstFile.string(), (*result)[1].string());

    std::remove(firstFile.string().c_str());
    std::remove(secondFile.string().c_str());
    std::filesystem::remove(directoryPath.string());
}
