/**
 * @file format_detector_test.cpp
 * @brief Unit tests for FormatDetector and LogFormat helpers.
 */

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "analysis.hpp"

using scope::analysis::FormatDetector;
using scope::analysis::LogFormat;
using scope::analysis::logFormatName;
using scope::analysis::parseLogFormat;

TEST(LogFormatTest, ParsesKnownNames)
{
    EXPECT_EQ(LogFormat::Auto, *parseLogFormat("auto"));
    EXPECT_EQ(LogFormat::PlainText, *parseLogFormat("plain"));
    EXPECT_EQ(LogFormat::PlainText, *parseLogFormat("TEXT"));
    EXPECT_EQ(LogFormat::JsonLines, *parseLogFormat("jsonl"));
    EXPECT_EQ(LogFormat::JsonLines, *parseLogFormat("ndjson"));
    EXPECT_FALSE(parseLogFormat("xml").has_value());
}

TEST(LogFormatTest, NamesAreStable)
{
    EXPECT_EQ("auto", logFormatName(LogFormat::Auto));
    EXPECT_EQ("plain", logFormatName(LogFormat::PlainText));
    EXPECT_EQ("jsonl", logFormatName(LogFormat::JsonLines));
    EXPECT_EQ("unknown", logFormatName(LogFormat::Unknown));
}

TEST(FormatDetectorTest, DetectsPlainText)
{
    const std::vector<std::string> lines = {
        "2026-07-11 10:00:01 INFO Application started",
        "2026-07-11 10:00:06 ERROR Connection refused",
    };

    const auto result = FormatDetector::detect(lines);

    EXPECT_FALSE(result.looksBinary);
    EXPECT_EQ(LogFormat::PlainText, result.format);
    EXPECT_EQ(2U, result.sampledLines);
}

TEST(FormatDetectorTest, DetectsJsonLines)
{
    const std::vector<std::string> lines = {
        R"({"level":"info","message":"started"})",
        R"({"level":"error","message":"failed"})",
        "not json",
    };

    const auto result = FormatDetector::detect(lines);

    EXPECT_FALSE(result.looksBinary);
    EXPECT_EQ(LogFormat::JsonLines, result.format);
}

TEST(FormatDetectorTest, DetectsBinaryNullByte)
{
    std::string binaryLine = "hello";
    binaryLine.push_back('\0');
    binaryLine.append("world");

    const auto result = FormatDetector::detect(std::vector<std::string>{binaryLine});

    EXPECT_TRUE(result.looksBinary);
    EXPECT_EQ(LogFormat::Unknown, result.format);
}

TEST(FormatDetectorTest, EmptySampleIsPlainText)
{
    const auto result = FormatDetector::detect(std::vector<std::string>{});

    EXPECT_FALSE(result.looksBinary);
    EXPECT_EQ(LogFormat::PlainText, result.format);
}
