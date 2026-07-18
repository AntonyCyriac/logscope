/**
 * @file path_test.cpp
 * @brief Unit tests for Path.
 */

#include <gtest/gtest.h>

#include "foundation.hpp"

using scope::foundation::Path;

TEST(PathTest, DefaultConstruction)
{
    const Path path;

    EXPECT_TRUE(path.isEmpty());
}

TEST(PathTest, Construction)
{
    const Path path("samples/sample.log");

    EXPECT_EQ("samples/sample.log", path.string());
    EXPECT_FALSE(path.isEmpty());
}

TEST(PathTest, Filename)
{
    const Path path("samples/sample.log");

    EXPECT_EQ("sample.log", path.filename().string());
}

TEST(PathTest, Extension)
{
    const Path path("samples/sample.log");

    EXPECT_EQ(".log", path.extension().string());
}

TEST(PathTest, Append)
{
    const Path base("samples");
    const Path combined = base.append("sample.log");

    EXPECT_EQ("sample.log", combined.filename().string());
    EXPECT_FALSE(combined.isEmpty());
}

TEST(PathTest, Equality)
{
    EXPECT_EQ(Path("a/b"), Path("a/b"));
    EXPECT_NE(Path("a/b"), Path("a/c"));
}
