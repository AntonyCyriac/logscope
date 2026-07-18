/**
 * @file time_test.cpp
 * @brief Unit tests for Time.
 */

#include <gtest/gtest.h>

#include "foundation.hpp"

using scope::foundation::ErrorCode;
using scope::foundation::Time;

TEST(TimeTest, DefaultConstruction)
{
    const Time time;

    EXPECT_EQ(0, time.hour());
    EXPECT_EQ(0, time.minute());
    EXPECT_EQ(0, time.second());
    EXPECT_EQ(0, time.nanosecond());
    EXPECT_EQ("00:00:00", time.toString());
}

TEST(TimeTest, ParameterizedConstruction)
{
    const Time time(14, 30, 45, 123456789);

    EXPECT_EQ(14, time.hour());
    EXPECT_EQ(30, time.minute());
    EXPECT_EQ(45, time.second());
    EXPECT_EQ(123456789, time.nanosecond());
}

TEST(TimeTest, ParseValid)
{
    auto result = Time::parse("14:30:45");

    ASSERT_TRUE(result.hasValue());
    EXPECT_EQ(14, result->hour());
    EXPECT_EQ(30, result->minute());
    EXPECT_EQ(45, result->second());
}

TEST(TimeTest, ParseWithFraction)
{
    auto result = Time::parse("14:30:45.5");

    ASSERT_TRUE(result.hasValue());
    EXPECT_EQ(500000000, result->nanosecond());
}

TEST(TimeTest, ParseInvalidFormat)
{
    auto result = Time::parse("14-30-45");

    ASSERT_TRUE(result.hasError());
    EXPECT_EQ(ErrorCode::InvalidArgument, result.error().code());
}

TEST(TimeTest, ParseOutOfRange)
{
    auto result = Time::parse("24:00:00");

    ASSERT_TRUE(result.hasError());
    EXPECT_EQ("Hour out of range.", result.error().message());
}

TEST(TimeTest, Equality)
{
    const Time first(10, 20, 30);
    const Time second(10, 20, 30);
    const Time third(10, 20, 31);

    EXPECT_EQ(first, second);
    EXPECT_NE(first, third);
}

TEST(TimeTest, Ordering)
{
    EXPECT_LT(Time(10, 0, 0), Time(11, 0, 0));
    EXPECT_LT(Time(10, 0, 0), Time(10, 1, 0));
}
