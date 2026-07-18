/**
 * @file duration_test.cpp
 * @brief Unit tests for Duration.
 */

#include <gtest/gtest.h>

#include "foundation.hpp"

using scope::foundation::Duration;
using scope::foundation::ErrorCode;

TEST(DurationTest, DefaultConstruction)
{
    const Duration duration;

    EXPECT_EQ(0, duration.hours());
    EXPECT_EQ(0, duration.minutes());
    EXPECT_EQ(0, duration.seconds());
    EXPECT_EQ(0, duration.nanoseconds());
    EXPECT_EQ(0, duration.totalNanoseconds());
    EXPECT_EQ("0:00:00", duration.toString());
}

TEST(DurationTest, ParameterizedConstruction)
{
    const Duration duration(1, 30, 45, 123456789);

    EXPECT_EQ(1, duration.hours());
    EXPECT_EQ(30, duration.minutes());
    EXPECT_EQ(45, duration.seconds());
    EXPECT_EQ(123456789, duration.nanoseconds());
}

TEST(DurationTest, FromNanoseconds)
{
    auto result = Duration::fromNanoseconds(1500000000LL);

    ASSERT_TRUE(result.hasValue());
    EXPECT_EQ(1, result->seconds());
    EXPECT_EQ(1500000000LL, result->totalNanoseconds());
}

TEST(DurationTest, FromSeconds)
{
    auto result = Duration::fromSeconds(90);

    ASSERT_TRUE(result.hasValue());
    EXPECT_EQ(1, result->minutes());
    EXPECT_EQ(30, result->seconds());
}

TEST(DurationTest, FromNanosecondsNegative)
{
    auto result = Duration::fromNanoseconds(-1);

    ASSERT_TRUE(result.hasError());
    EXPECT_EQ(ErrorCode::InvalidArgument, result.error().code());
}

TEST(DurationTest, ParseValid)
{
    auto result = Duration::parse("1:30:45");

    ASSERT_TRUE(result.hasValue());
    EXPECT_EQ(1, result->hours());
    EXPECT_EQ(30, result->minutes());
    EXPECT_EQ(45, result->seconds());
}

TEST(DurationTest, ParseLargeHours)
{
    auto result = Duration::parse("100:00:00");

    ASSERT_TRUE(result.hasValue());
    EXPECT_EQ(100, result->hours());
}

TEST(DurationTest, ParseWithFraction)
{
    auto result = Duration::parse("0:00:01.5");

    ASSERT_TRUE(result.hasValue());
    EXPECT_EQ(500000000, result->nanoseconds());
}

TEST(DurationTest, ParseInvalidFormat)
{
    auto result = Duration::parse("1-30-45");

    ASSERT_TRUE(result.hasError());
    EXPECT_EQ(ErrorCode::InvalidArgument, result.error().code());
}

TEST(DurationTest, ParseOutOfRangeMinutes)
{
    auto result = Duration::parse("1:60:00");

    ASSERT_TRUE(result.hasError());
    EXPECT_EQ("Minutes out of range.", result.error().message());
}

TEST(DurationTest, Equality)
{
    const Duration first(1, 0, 0);
    const Duration second(1, 0, 0);
    const Duration third(2, 0, 0);

    EXPECT_EQ(first, second);
    EXPECT_NE(first, third);
}

TEST(DurationTest, Ordering)
{
    EXPECT_LT(Duration(1, 0, 0), Duration(2, 0, 0));
    EXPECT_LT(Duration(0, 1, 0), Duration(0, 2, 0));
}

TEST(DurationTest, ToString)
{
    const Duration duration(10, 5, 7);

    EXPECT_EQ("10:05:07", duration.toString());
}
