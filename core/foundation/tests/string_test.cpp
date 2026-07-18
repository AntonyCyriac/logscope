/**
 * @file string_test.cpp
 * @brief Unit tests for string utilities.
 */

#include <gtest/gtest.h>

#include "foundation.hpp"

using scope::foundation::endsWith;
using scope::foundation::isBlank;
using scope::foundation::split;
using scope::foundation::startsWith;
using scope::foundation::toLower;
using scope::foundation::toUpper;
using scope::foundation::trim;
using scope::foundation::trimLeft;
using scope::foundation::trimRight;

TEST(StringTest, IsBlank)
{
    EXPECT_TRUE(isBlank(""));
    EXPECT_TRUE(isBlank("   "));
    EXPECT_FALSE(isBlank("a"));
}

TEST(StringTest, Trim)
{
    EXPECT_EQ("hello", trim("  hello  "));
    EXPECT_EQ("hello", trimLeft("  hello"));
    EXPECT_EQ("hello", trimRight("hello  "));
}

TEST(StringTest, CaseConversion)
{
    EXPECT_EQ("hello", toLower("HeLLo"));
    EXPECT_EQ("HELLO", toUpper("HeLLo"));
}

TEST(StringTest, StartsWith)
{
    EXPECT_TRUE(startsWith("logscope", "log"));
    EXPECT_FALSE(startsWith("logscope", "scope"));
}

TEST(StringTest, EndsWith)
{
    EXPECT_TRUE(endsWith("logscope", "scope"));
    EXPECT_FALSE(endsWith("logscope", "log"));
}

TEST(StringTest, Split)
{
    const auto parts = split("a,b,c", ',');

    ASSERT_EQ(3U, parts.size());
    EXPECT_EQ("a", parts[0]);
    EXPECT_EQ("b", parts[1]);
    EXPECT_EQ("c", parts[2]);
}

TEST(StringTest, SplitEmptyTrailing)
{
    const auto parts = split("a,b,", ',');

    ASSERT_EQ(3U, parts.size());
    EXPECT_EQ("a", parts[0]);
    EXPECT_EQ("b", parts[1]);
    EXPECT_EQ("", parts[2]);
}
