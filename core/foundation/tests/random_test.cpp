/**
 * @file random_test.cpp
 * @brief Unit tests for Random.
 */

#include <gtest/gtest.h>

#include "foundation.hpp"

using scope::foundation::ErrorCode;
using scope::foundation::Random;

TEST(RandomTest, SeededSequenceIsDeterministic)
{
    Random first(42);
    Random second(42);

    EXPECT_EQ(first.nextUInt32(), second.nextUInt32());
    EXPECT_EQ(first.nextUInt64(), second.nextUInt64());
}

TEST(RandomTest, DifferentSeedsProduceDifferentValues)
{
    Random first(1);
    Random second(2);

    EXPECT_NE(first.nextUInt32(), second.nextUInt32());
}

TEST(RandomTest, NextIntWithinRange)
{
    Random random(99);

    for (int index = 0; index < 100; ++index)
    {
        auto result = random.nextInt(10, 20);

        ASSERT_TRUE(result.hasValue());
        EXPECT_GE(*result, 10);
        EXPECT_LE(*result, 20);
    }
}

TEST(RandomTest, NextIntInvalidRange)
{
    Random random(1);

    auto result = random.nextInt(20, 10);

    ASSERT_TRUE(result.hasError());
    EXPECT_EQ(ErrorCode::InvalidArgument, result.error().code());
}

TEST(RandomTest, NextBool)
{
    Random random(7);

    const bool value = random.nextBool();

    EXPECT_TRUE(value == true || value == false);
}

TEST(RandomTest, CreateDoesNotThrow)
{
    EXPECT_NO_THROW(static_cast<void>(Random::create()));
}
