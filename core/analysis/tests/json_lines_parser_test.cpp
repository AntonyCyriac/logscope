/**
 * @file json_lines_parser_test.cpp
 * @brief Unit tests for JSON Lines parsing.
 */

#include <gtest/gtest.h>

#include "json_field_mapping.hpp"
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
    ASSERT_EQ(2U, result.topLevelFieldValues.size());
    EXPECT_EQ("level", result.topLevelFieldValues[0].first);
    EXPECT_EQ("error", result.topLevelFieldValues[0].second);
}

TEST(JsonLinesParserTest, CapturesTopLevelStringAndNumberFields)
{
    const auto result =
        JsonLinesParser::parse(R"({"service":"PCF","count":42,"nested":{"ignored":true}})");

    ASSERT_EQ(JsonLineParseOutcome::Valid, result.outcome);
    ASSERT_EQ(3U, result.topLevelKeys.size());
    ASSERT_EQ(2U, result.topLevelFieldValues.size());
    EXPECT_EQ("service", result.topLevelFieldValues[0].first);
    EXPECT_EQ("PCF", result.topLevelFieldValues[0].second);
    EXPECT_EQ("count", result.topLevelFieldValues[1].first);
    EXPECT_EQ("42", result.topLevelFieldValues[1].second);
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
    EXPECT_EQ("ok", result.messageValue);
    EXPECT_EQ("log", result.topLevelKeys[0]);
}

TEST(JsonLinesParserTest, ParsesTimestampAndMessageFields)
{
    const auto result = JsonLinesParser::parse(
        R"({"timestamp":"2026-07-21T10:00:01Z","message":"started","level":"info"})");

    ASSERT_EQ(JsonLineParseOutcome::Valid, result.outcome);
    EXPECT_EQ("2026-07-21T10:00:01Z", result.timestampValue);
    EXPECT_EQ("started", result.messageValue);
    EXPECT_EQ("info", result.levelValue);
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

TEST(JsonLinesParserTest, UsesConfiguredFieldMapping)
{
    scope::analysis::JsonFieldMapping mapping;
    mapping.timestampField = "@ts";
    mapping.levelField = "severity";

    const auto result = JsonLinesParser::parse(R"({"@ts":"2026-07-21T10:00:01Z","severity":"error","message":"fail"})",
                                             mapping);

    ASSERT_EQ(JsonLineParseOutcome::Valid, result.outcome);
    EXPECT_EQ("2026-07-21T10:00:01Z", result.timestampValue);
    EXPECT_EQ("error", result.levelValue);
}

TEST(JsonLinesParserTest, MapsJsonLevelValues)
{
    EXPECT_EQ(DetectedLogLevel::Error, detectLogLevelFromJsonField("ERROR"));
    EXPECT_EQ(DetectedLogLevel::Warn, detectLogLevelFromJsonField("warning"));
    EXPECT_EQ(DetectedLogLevel::Info, detectLogLevelFromJsonField("debug"));
    EXPECT_EQ(DetectedLogLevel::Other, detectLogLevelFromJsonField("trace"));
}
