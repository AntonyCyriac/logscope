/**
 * @file json_lines_parser_test.cpp
 * @brief Unit tests for JSON Lines parsing.
 */

#include <gtest/gtest.h>

#include "json_lines_parser.hpp"

using scope::analysis::DetectedLogLevel;
using scope::analysis::JsonLineParseOutcome;
using scope::analysis::JsonLinesParser;
using scope::analysis::detectLogLevelFromJsonField;

TEST(JsonLinesParserTest, ParsesBlankLine)
{
    const auto result = JsonLinesParser::parse("   ");

    EXPECT_EQ(JsonLineParseOutcome::Blank, result.outcome);
}

TEST(JsonLinesParserTest, ParsesValidObjectWithLevelField)
{
    const auto result = JsonLinesParser::parse(R"({"level":"error","message":"boom"})");

    ASSERT_EQ(JsonLineParseOutcome::Valid, result.outcome);
    EXPECT_EQ("error", result.levelValue);
    ASSERT_EQ(2U, result.topLevelKeys.size());
    EXPECT_EQ("level", result.topLevelKeys[0]);
    EXPECT_EQ("message", result.topLevelKeys[1]);
}

TEST(JsonLinesParserTest, ParsesSeverityField)
{
    const auto result = JsonLinesParser::parse(R"({"severity":"warning","message":"slow"})");

    ASSERT_EQ(JsonLineParseOutcome::Valid, result.outcome);
    EXPECT_EQ("warning", result.levelValue);
}

TEST(JsonLinesParserTest, ParsesNestedLogLevelField)
{
    const auto result = JsonLinesParser::parse(R"({"log":{"level":"info"},"message":"ok"})");

    ASSERT_EQ(JsonLineParseOutcome::Valid, result.outcome);
    EXPECT_EQ("info", result.levelValue);
    EXPECT_EQ("log", result.topLevelKeys[0]);
}

TEST(JsonLinesParserTest, RejectsInvalidJsonLine)
{
    const auto result = JsonLinesParser::parse("not json");

    EXPECT_EQ(JsonLineParseOutcome::Invalid, result.outcome);
}

TEST(JsonLinesParserTest, RejectsTrailingContent)
{
    const auto result = JsonLinesParser::parse(R"({"level":"info"} extra)");

    EXPECT_EQ(JsonLineParseOutcome::Invalid, result.outcome);
}

TEST(JsonLinesParserTest, MapsJsonLevelValues)
{
    EXPECT_EQ(DetectedLogLevel::Error, detectLogLevelFromJsonField("ERROR"));
    EXPECT_EQ(DetectedLogLevel::Warn, detectLogLevelFromJsonField("warning"));
    EXPECT_EQ(DetectedLogLevel::Info, detectLogLevelFromJsonField("debug"));
    EXPECT_EQ(DetectedLogLevel::Other, detectLogLevelFromJsonField("trace"));
}
