/**
 * @file search_query_test.cpp
 * @brief Unit tests for SearchQuery.
 */

#include <gtest/gtest.h>

#include "search_query.hpp"

using scope::search::CaseSensitivity;
using scope::search::SearchMode;
using scope::search::SearchQuery;

TEST(SearchQueryTest, MatchAllIsInactive)
{
    const SearchQuery query = SearchQuery::matchAll();

    EXPECT_FALSE(query.isActive());
    EXPECT_TRUE(query.toString().empty());
}

TEST(SearchQueryTest, BuildsBooleanExpression)
{
    SearchQuery query = SearchQuery::andQuery(SearchQuery::term("error"), SearchQuery::term("timeout"));

    EXPECT_TRUE(query.isActive());
    EXPECT_EQ("(error AND timeout)", query.toString());
}

TEST(SearchQueryTest, QuotesTermsWithSpaces)
{
    const SearchQuery query = SearchQuery::term("connection refused");

    EXPECT_EQ("\"connection refused\"", query.toString());
}

TEST(SearchQueryTest, SupportsRegexTerm)
{
    const SearchQuery query =
        SearchQuery::term("error.*", SearchMode::Regex, CaseSensitivity::Sensitive);

    EXPECT_EQ(SearchMode::Regex, query.mode());
    EXPECT_EQ(CaseSensitivity::Sensitive, query.caseSensitivity());
}
