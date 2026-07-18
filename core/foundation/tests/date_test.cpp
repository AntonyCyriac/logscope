/**
 * @file date_test.cpp
 * @brief Unit tests for Date.
 */

#include <gtest/gtest.h>

#include "foundation.hpp"

using scope::foundation::Date;
using scope::foundation::ErrorCode;

TEST(DateTest, DefaultConstruction)
{
    const Date date;

    EXPECT_EQ(1970, date.year());
    EXPECT_EQ(1, date.month());
    EXPECT_EQ(1, date.day());
    EXPECT_EQ("1970-01-01", date.toString());
}

TEST(DateTest, ParseValid)
{
    auto result = Date::parse("2026-07-18");

    ASSERT_TRUE(result.hasValue());
    EXPECT_EQ("2026-07-18", result->toString());
}

TEST(DateTest, ParseLeapYear)
{
    auto result = Date::parse("2024-02-29");

    ASSERT_TRUE(result.hasValue());
}

TEST(DateTest, ParseInvalidDay)
{
    auto result = Date::parse("2023-02-29");

    ASSERT_TRUE(result.hasError());
    EXPECT_EQ("Day out of range.", result.error().message());
}

TEST(DateTest, Equality)
{
    EXPECT_EQ(Date(2026, 7, 18), Date(2026, 7, 18));
    EXPECT_NE(Date(2026, 7, 18), Date(2026, 7, 19));
}

TEST(DateTest, Ordering)
{
    EXPECT_LT(Date(2026, 1, 1), Date(2026, 12, 31));
}
