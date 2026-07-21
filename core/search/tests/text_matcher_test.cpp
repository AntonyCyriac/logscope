/**
 * @file text_matcher_test.cpp
 * @brief Unit tests for text matching helpers.
 */

#include <gtest/gtest.h>

#include "text_matcher.hpp"

using scope::search::CaseSensitivity;
using scope::search::textContains;

TEST(TextMatcherTest, FindsCaseInsensitiveSubstring)
{
    EXPECT_TRUE(textContains("Connection REFUSED", "refused", CaseSensitivity::Insensitive));
}

TEST(TextMatcherTest, RespectsCaseSensitiveMode)
{
    EXPECT_FALSE(textContains("Connection REFUSED", "refused", CaseSensitivity::Sensitive));
    EXPECT_TRUE(textContains("Connection refused", "refused", CaseSensitivity::Sensitive));
}

TEST(TextMatcherTest, EmptyNeedleMatchesEverything)
{
    EXPECT_TRUE(textContains("anything", "", CaseSensitivity::Insensitive));
}
