/**
 * @file time_range_filter_test.cpp
 * @brief Unit tests for TimeRangeFilter.
 */

#include <gtest/gtest.h>

#include "foundation/timestamp.hpp"
#include "line_index.hpp"
#include "time_range_filter.hpp"

using scope::analysis::IndexedLine;
using scope::foundation::Timestamp;
using scope::investigation::TimeRangeFilter;

TEST(TimeRangeFilterTest, MatchesTimestampWithinRange)
{
    const auto earliest = Timestamp::parse("2026-07-11T10:00:05");
    const auto latest = Timestamp::parse("2026-07-11T10:00:15");
    const auto inside = Timestamp::parse("2026-07-11T10:00:10");

    ASSERT_TRUE(earliest.hasValue());
    ASSERT_TRUE(latest.hasValue());
    ASSERT_TRUE(inside.hasValue());

    const TimeRangeFilter filter =
        TimeRangeFilter::any().withEarliest(*earliest).withLatest(*latest);

    IndexedLine line;
    line.timestamp = *inside;

    EXPECT_TRUE(filter.matches(line));
}

TEST(TimeRangeFilterTest, RejectsTimestampOutsideRange)
{
    const auto earliest = Timestamp::parse("2026-07-11T10:00:05");
    const auto outside = Timestamp::parse("2026-07-11T10:00:01");

    ASSERT_TRUE(earliest.hasValue());
    ASSERT_TRUE(outside.hasValue());

    const TimeRangeFilter filter = TimeRangeFilter::any().withEarliest(*earliest);

    IndexedLine line;
    line.timestamp = *outside;

    EXPECT_FALSE(filter.matches(line));
}
