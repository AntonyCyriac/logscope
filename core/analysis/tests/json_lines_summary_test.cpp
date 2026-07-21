/**
 * @file json_lines_summary_test.cpp
 * @brief Unit tests for JsonLinesSummary.
 */

#include <gtest/gtest.h>

#include "json_lines_summary.hpp"

using scope::analysis::JsonLinesSummary;

TEST(JsonLinesSummaryTest, TracksValidLinesAndKeys)
{
    JsonLinesSummary summary;

    summary.recordValidLine({"level", "message"});
    summary.recordValidLine({"level", "timestamp"});

    EXPECT_EQ(2U, summary.validLines());
    EXPECT_EQ(0U, summary.parseFailures());

    const auto topKeys = summary.topLevelKeys();

    ASSERT_EQ(3U, topKeys.size());
    EXPECT_EQ("level", topKeys[0].first);
    EXPECT_EQ(2U, topKeys[0].second);
    EXPECT_EQ("message", topKeys[1].first);
    EXPECT_EQ(1U, topKeys[1].second);
    EXPECT_EQ("timestamp", topKeys[2].first);
    EXPECT_EQ(1U, topKeys[2].second);
}

TEST(JsonLinesSummaryTest, TracksParseFailures)
{
    JsonLinesSummary summary;

    summary.recordParseFailure();
    summary.recordParseFailure();

    EXPECT_EQ(2U, summary.parseFailures());
}
