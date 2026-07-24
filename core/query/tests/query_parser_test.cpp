/**
 * @file query_parser_test.cpp
 */

#include <gtest/gtest.h>

#include "query_parser.hpp"

using scope::query::ComparisonOperator;
using scope::query::FunctionKind;
using scope::query::QueryNode;
using scope::query::QueryValue;
using scope::query::parseFilterQuery;

TEST(QueryParserTest, ParsesLevelComparison)
{
    const auto parsed = parseFilterQuery("level == ERROR");

    ASSERT_TRUE(parsed);
    EXPECT_EQ(QueryNode::Kind::Comparison, parsed->kind());
    EXPECT_EQ("level", parsed->field());
    EXPECT_EQ(ComparisonOperator::Equal, parsed->comparisonOperator());
    EXPECT_EQ(QueryValue::Kind::Level, parsed->value().kind());
}

TEST(QueryParserTest, ParsesContainsFunction)
{
    const auto parsed = parseFilterQuery(R"(contains(message, "timeout"))");

    ASSERT_TRUE(parsed);
    EXPECT_EQ(QueryNode::Kind::FunctionCall, parsed->kind());
    EXPECT_EQ(FunctionKind::Contains, parsed->functionKind());
    EXPECT_EQ("message", parsed->field());
    EXPECT_EQ("timeout", parsed->argument());
}

TEST(QueryParserTest, ParsesBooleanComposition)
{
    const auto parsed = parseFilterQuery(R"(level == ERROR AND contains(message, "refused"))");

    ASSERT_TRUE(parsed);
    EXPECT_EQ(QueryNode::Kind::And, parsed->kind());
    ASSERT_NE(nullptr, parsed->left());
    ASSERT_NE(nullptr, parsed->right());
}

TEST(QueryParserTest, ParsesHasKeyFunction)
{
    const auto parsed = parseFilterQuery(R"(hasKey("correlationId"))");

    ASSERT_TRUE(parsed);
    EXPECT_EQ(QueryNode::Kind::FunctionCall, parsed->kind());
    EXPECT_EQ(FunctionKind::HasKey, parsed->functionKind());
    EXPECT_EQ("correlationId", parsed->argument());
}

TEST(QueryParserTest, ParsesTimeComparison)
{
    const auto parsed = parseFilterQuery(R"(time >= "2026-07-11 10:00:00")");

    ASSERT_TRUE(parsed);
    EXPECT_EQ(QueryNode::Kind::Comparison, parsed->kind());
    EXPECT_EQ("time", parsed->field());
}

TEST(QueryParserTest, ParsesNotExpression)
{
    const auto parsed = parseFilterQuery("NOT level == INFO");

    ASSERT_TRUE(parsed);
    EXPECT_EQ(QueryNode::Kind::Not, parsed->kind());
}

TEST(QueryParserTest, ParsesOrExpression)
{
    const auto parsed = parseFilterQuery("level == ERROR OR level == WARNING");

    ASSERT_TRUE(parsed);
    EXPECT_EQ(QueryNode::Kind::Or, parsed->kind());
}

TEST(QueryParserTest, RejectsInvalidExpression)
{
    const auto parsed = parseFilterQuery("level == ");

    EXPECT_FALSE(parsed);
}

TEST(QueryParserTest, ParsesParenthesizedExpression)
{
    const auto parsed = parseFilterQuery("(level == ERROR OR level == WARNING) AND contains(message, \"timeout\")");

    ASSERT_TRUE(parsed);
    EXPECT_EQ(QueryNode::Kind::And, parsed->kind());
}

TEST(QueryParserTest, ParsesNotEqualOperator)
{
    const auto parsed = parseFilterQuery("level != INFO");

    ASSERT_TRUE(parsed);
    EXPECT_EQ(QueryNode::Kind::Comparison, parsed->kind());
    EXPECT_EQ(ComparisonOperator::NotEqual, parsed->comparisonOperator());
}

TEST(QueryParserTest, ParsesTimestampAlias)
{
    const auto parsed = parseFilterQuery(R"(timestamp >= "2026-07-11 10:00:00")");

    ASSERT_TRUE(parsed);
    EXPECT_EQ("timestamp", parsed->field());
}

TEST(QueryParserTest, ParsesLineNumberComparison)
{
    const auto parsed = parseFilterQuery("line > 10");

    ASSERT_TRUE(parsed);
    EXPECT_EQ("line", parsed->field());
    EXPECT_EQ(ComparisonOperator::Greater, parsed->comparisonOperator());
}

TEST(QueryParserTest, ParsesContentFieldComparison)
{
    const auto parsed = parseFilterQuery(R"(content == "full line text")");

    ASSERT_TRUE(parsed);
    EXPECT_EQ("content", parsed->field());
}
