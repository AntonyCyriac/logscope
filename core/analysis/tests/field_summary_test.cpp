/**
 * @file field_summary_test.cpp
 * @brief Unit tests for FieldSummary.
 */

#include <gtest/gtest.h>

#include "field_summary.hpp"

using scope::analysis::FieldSummary;
using scope::foundation::Timestamp;

TEST(FieldSummaryTest, TracksTimestampRange)
{
    FieldSummary summary;

    const auto first = Timestamp::parse("2026-07-11T10:00:01");
    const auto second = Timestamp::parse("2026-07-11T10:00:25");

    ASSERT_TRUE(first.hasValue());
    ASSERT_TRUE(second.hasValue());

    summary.recordTimestamp(*first);
    summary.recordTimestamp(*second);

    ASSERT_TRUE(summary.earliestTimestamp().has_value());
    ASSERT_TRUE(summary.latestTimestamp().has_value());
    EXPECT_EQ(*first, *summary.earliestTimestamp());
    EXPECT_EQ(*second, *summary.latestTimestamp());
    EXPECT_EQ(2U, summary.linesWithTimestamp());
}

TEST(FieldSummaryTest, TracksTopMessages)
{
    FieldSummary summary;

    summary.recordMessage("Connection refused");
    summary.recordMessage("Connection refused");
    summary.recordMessage("Application started");

    EXPECT_EQ(3U, summary.linesWithMessage());

    const auto topMessages = summary.topMessages(2U);

    ASSERT_EQ(2U, topMessages.size());
    EXPECT_EQ("Connection refused", topMessages[0].first);
    EXPECT_EQ(2U, topMessages[0].second);
}
