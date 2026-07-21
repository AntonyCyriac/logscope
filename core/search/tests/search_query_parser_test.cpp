/**
 * @file search_query_parser_test.cpp
 * @brief Unit tests for search query parsing.
 */

#include <gtest/gtest.h>

#include "search_query_parser.hpp"

using scope::search::parseRegexQuery;
using scope::search::parseSearchQuery;

TEST(SearchQueryParserTest, ParsesSimpleTerm)
{
    const auto query = parseSearchQuery("refused");

    ASSERT_TRUE(query.hasValue());
    EXPECT_EQ("refused", query->term());
}

TEST(SearchQueryParserTest, ParsesBooleanExpression)
{
    const auto query = parseSearchQuery("(error OR warning) AND timeout");

    ASSERT_TRUE(query.hasValue());
    EXPECT_EQ("((error OR warning) AND timeout)", query->toString());
}

TEST(SearchQueryParserTest, ParsesQuotedPhrase)
{
    const auto query = parseSearchQuery("\"connection refused\"");

    ASSERT_TRUE(query.hasValue());
    EXPECT_EQ("\"connection refused\"", query->toString());
}

TEST(SearchQueryParserTest, RejectsUnterminatedQuote)
{
    const auto query = parseSearchQuery("\"connection refused");

    EXPECT_FALSE(query.hasValue());
}

TEST(SearchQueryParserTest, RejectsInvalidRegex)
{
    const auto query = parseRegexQuery("[unclosed", scope::search::CaseSensitivity::Insensitive);

    EXPECT_FALSE(query.hasValue());
}
