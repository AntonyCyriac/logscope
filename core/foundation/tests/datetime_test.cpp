/**
 * @file datetime_test.cpp
 * @brief Unit tests for DateTime.
 */

#include <gtest/gtest.h>

#include "foundation.hpp"

using scope::foundation::Date;
using scope::foundation::DateTime;
using scope::foundation::Time;

TEST(DateTimeTest, DefaultConstruction)
{
    const DateTime dateTime;

    EXPECT_EQ("1970-01-01T00:00:00", dateTime.toString());
}

TEST(DateTimeTest, Construction)
{
    const DateTime dateTime(Date(2026, 7, 18), Time(14, 30, 0));

    EXPECT_EQ(2026, dateTime.date().year());
    EXPECT_EQ(14, dateTime.time().hour());
}

TEST(DateTimeTest, ParseValid)
{
    auto result = DateTime::parse("2026-07-18T14:30:45");

    ASSERT_TRUE(result.hasValue());
    EXPECT_EQ("2026-07-18T14:30:45", result->toString());
}

TEST(DateTimeTest, ParseInvalid)
{
    auto result = DateTime::parse("2026-07-18 14:30:45");

    ASSERT_TRUE(result.hasError());
}

TEST(DateTimeTest, Equality)
{
    const DateTime first(Date(2026, 7, 18), Time(10, 0, 0));
    const DateTime second(Date(2026, 7, 18), Time(10, 0, 0));

    EXPECT_EQ(first, second);
}

TEST(DateTimeTest, Ordering)
{
    EXPECT_LT(DateTime(Date(2026, 1, 1), Time(0, 0, 0)),
              DateTime(Date(2026, 1, 2), Time(0, 0, 0)));
}
