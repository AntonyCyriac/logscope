/**
 * @file timestamp_test.cpp
 * @brief Unit tests for Timestamp.
 */

#include <gtest/gtest.h>

#include "foundation.hpp"

using scope::foundation::Date;
using scope::foundation::DateTime;
using scope::foundation::Duration;
using scope::foundation::ErrorCode;
using scope::foundation::Time;
using scope::foundation::Timestamp;

TEST(TimestampTest, DefaultConstruction)
{
    const Timestamp timestamp;

    EXPECT_EQ(0, timestamp.unixNanoseconds());
    EXPECT_EQ(0, timestamp.unixSeconds());
    EXPECT_EQ("1970-01-01T00:00:00", timestamp.toString());
}

TEST(TimestampTest, FromUnixSeconds)
{
    const Timestamp timestamp = Timestamp::fromUnixSeconds(1);

    EXPECT_EQ(1000000000LL, timestamp.unixNanoseconds());
    EXPECT_EQ(1, timestamp.unixSeconds());
}

TEST(TimestampTest, FromDateTime)
{
    const DateTime dateTime(Date(1970, 1, 1), Time(0, 0, 1));
    auto result = Timestamp::fromDateTime(dateTime);

    ASSERT_TRUE(result.hasValue());
    EXPECT_EQ(1000000000LL, result->unixNanoseconds());
}

TEST(TimestampTest, ParseValid)
{
    auto result = Timestamp::parse("2024-06-15T14:30:45");

    ASSERT_TRUE(result.hasValue());
    EXPECT_EQ("2024-06-15T14:30:45", result->toString());
}

TEST(TimestampTest, ParseInvalid)
{
    auto result = Timestamp::parse("invalid");

    ASSERT_TRUE(result.hasError());
    EXPECT_EQ(ErrorCode::InvalidArgument, result.error().code());
}

TEST(TimestampTest, ToDateTimeRoundTrip)
{
    const DateTime original(Date(2024, 6, 15), Time(14, 30, 45, 123456789));
    auto timestampResult = Timestamp::fromDateTime(original);

    ASSERT_TRUE(timestampResult.hasValue());

    auto roundTrip = timestampResult->toDateTime();

    ASSERT_TRUE(roundTrip.hasValue());
    EXPECT_EQ(original, *roundTrip);
}

TEST(TimestampTest, AddDuration)
{
    const Timestamp timestamp = Timestamp::fromUnixSeconds(10);
    const Duration duration(0, 0, 5);

    EXPECT_EQ(15, (timestamp + duration).unixSeconds());
}

TEST(TimestampTest, SubtractDuration)
{
    const Timestamp timestamp = Timestamp::fromUnixSeconds(10);
    const Duration duration(0, 0, 3);

    EXPECT_EQ(7, (timestamp - duration).unixSeconds());
}

TEST(TimestampTest, Difference)
{
    const Timestamp earlier = Timestamp::fromUnixSeconds(5);
    const Timestamp later = Timestamp::fromUnixSeconds(15);

    auto result = Timestamp::difference(later, earlier);

    ASSERT_TRUE(result.hasValue());
    EXPECT_EQ(10, result->seconds());
}

TEST(TimestampTest, DifferenceInvalidOrder)
{
    const Timestamp earlier = Timestamp::fromUnixSeconds(15);
    const Timestamp later = Timestamp::fromUnixSeconds(5);

    auto result = Timestamp::difference(later, earlier);

    ASSERT_TRUE(result.hasError());
    EXPECT_EQ(ErrorCode::InvalidArgument, result.error().code());
}

TEST(TimestampTest, Equality)
{
    const Timestamp first = Timestamp::fromUnixSeconds(42);
    const Timestamp second = Timestamp::fromUnixSeconds(42);
    const Timestamp third = Timestamp::fromUnixSeconds(43);

    EXPECT_EQ(first, second);
    EXPECT_NE(first, third);
}

TEST(TimestampTest, Ordering)
{
    EXPECT_LT(Timestamp::fromUnixSeconds(1), Timestamp::fromUnixSeconds(2));
}
