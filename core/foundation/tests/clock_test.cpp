/**
 * @file clock_test.cpp
 * @brief Unit tests for Clock.
 */

#include <gtest/gtest.h>

#include "foundation.hpp"

using scope::foundation::Clock;
using scope::foundation::Timestamp;

TEST(ClockTest, NowIsAfterEpoch)
{
    EXPECT_GT(Clock::now().unixNanoseconds(), 0);
}

TEST(ClockTest, NowIsNonDecreasing)
{
    const Timestamp first = Clock::now();
    const Timestamp second = Clock::now();

    EXPECT_LE(first, second);
}

TEST(ClockTest, NowProducesParseableDateTime)
{
    const auto dateTime = Clock::now().toDateTime();

    ASSERT_TRUE(dateTime.hasValue());
    EXPECT_GE(dateTime->date().year(), 1970);
}
