/**
 * @file hash_test.cpp
 * @brief Unit tests for hash utilities.
 */

#include <gtest/gtest.h>

#include "foundation.hpp"

using scope::foundation::hashBytes;
using scope::foundation::hashCombine;
using scope::foundation::hashString;

TEST(HashTest, HashStringDeterministic)
{
    EXPECT_EQ(hashString("logscope"), hashString("logscope"));
    EXPECT_NE(hashString("logscope"), hashString("LogScope"));
}

TEST(HashTest, HashStringEmpty)
{
    EXPECT_EQ(hashString(""), hashBytes(nullptr, 0));
}

TEST(HashTest, HashBytes)
{
    const char data[] = {'a', 'b', 'c'};

    EXPECT_EQ(hashString("abc"), hashBytes(data, 3));
}

TEST(HashTest, HashCombineChangesSeed)
{
    std::uint64_t seed = hashString("seed");
    const std::uint64_t original = seed;

    hashCombine(seed, hashString("value"));

    EXPECT_NE(original, seed);
}

TEST(HashTest, HashCombineOrderMatters)
{
    std::uint64_t firstSeed = 0;
    std::uint64_t secondSeed = 0;

    hashCombine(firstSeed, hashString("a"));
    hashCombine(firstSeed, hashString("b"));

    hashCombine(secondSeed, hashString("b"));
    hashCombine(secondSeed, hashString("a"));

    EXPECT_NE(firstSeed, secondSeed);
}
