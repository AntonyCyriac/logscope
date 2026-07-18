/**
 * @file uuid_test.cpp
 * @brief Unit tests for the Foundation Uuid type.
 */

#include <gtest/gtest.h>

#include "foundation.hpp"

using scope::foundation::ErrorCode;
using scope::foundation::Uuid;

TEST(UuidTest, DefaultConstruction)
{
    Uuid uuid;

    EXPECT_TRUE(uuid.isNil());
    EXPECT_EQ("00000000-0000-0000-0000-000000000000", uuid.toString());
}

TEST(UuidTest, IsNil)
{
    Uuid nil;

    auto parsed = Uuid::parse("00000000-0000-0000-0000-000000000000");

    ASSERT_TRUE(parsed.hasValue());

    EXPECT_TRUE(nil.isNil());
    EXPECT_TRUE(parsed->isNil());
    EXPECT_FALSE(Uuid::generate().isNil());
}

TEST(UuidTest, ToString)
{
    auto parsed = Uuid::parse("550e8400-e29b-41d4-a716-446655440000");

    ASSERT_TRUE(parsed.hasValue());

    EXPECT_EQ("550e8400-e29b-41d4-a716-446655440000", parsed->toString());
}

TEST(UuidTest, Equality)
{
    auto first = Uuid::parse("550e8400-e29b-41d4-a716-446655440000");
    auto second = Uuid::parse("550e8400-e29b-41d4-a716-446655440000");
    auto third = Uuid::parse("6ba7b810-9dad-11d1-80b4-00c04fd430c8");

    ASSERT_TRUE(first.hasValue());
    ASSERT_TRUE(second.hasValue());
    ASSERT_TRUE(third.hasValue());

    EXPECT_EQ(*first, *second);
    EXPECT_NE(*first, *third);
}

TEST(UuidTest, Ordering)
{
    auto lower = Uuid::parse("00000000-0000-0000-0000-000000000001");
    auto upper = Uuid::parse("00000000-0000-0000-0000-000000000002");

    ASSERT_TRUE(lower.hasValue());
    ASSERT_TRUE(upper.hasValue());

    EXPECT_LT(*lower, *upper);
    EXPECT_FALSE(*upper < *lower);
    EXPECT_FALSE(*lower < *lower);
}

TEST(UuidTest, ParseValid)
{
    auto result = Uuid::parse("550E8400-E29B-41D4-A716-446655440000");

    ASSERT_TRUE(result.hasValue());
    EXPECT_EQ("550e8400-e29b-41d4-a716-446655440000", result->toString());
}

TEST(UuidTest, ParseInvalidLength)
{
    auto result = Uuid::parse("550e8400-e29b-41d4-a716-44665544000");

    ASSERT_TRUE(result.hasError());
    EXPECT_EQ(ErrorCode::InvalidArgument, result.error().code());
    EXPECT_EQ("Invalid UUID length.", result.error().message());
}

TEST(UuidTest, ParseInvalidFormat)
{
    auto result = Uuid::parse("550e8400-e29b-41d4-a716_446655440000");

    ASSERT_TRUE(result.hasError());
    EXPECT_EQ(ErrorCode::InvalidArgument, result.error().code());
    EXPECT_EQ("Invalid UUID format.", result.error().message());
}

TEST(UuidTest, ParseInvalidCharacter)
{
    auto result = Uuid::parse("550e8400-e29b-41d4-a716-44665544000g");

    ASSERT_TRUE(result.hasError());
    EXPECT_EQ(ErrorCode::InvalidArgument, result.error().code());
    EXPECT_EQ("Invalid UUID character.", result.error().message());
}

TEST(UuidTest, Generate)
{
    Uuid first = Uuid::generate();
    Uuid second = Uuid::generate();

    EXPECT_FALSE(first.isNil());
    EXPECT_FALSE(second.isNil());
    EXPECT_NE(first, second);

    const std::string text = first.toString();

    EXPECT_EQ(36U, text.size());
    EXPECT_EQ('4', text[14]);
    EXPECT_TRUE(text[19] == '8' || text[19] == '9' || text[19] == 'a' || text[19] == 'b');

    auto roundTrip = Uuid::parse(text);

    ASSERT_TRUE(roundTrip.hasValue());
    EXPECT_EQ(first, *roundTrip);
}
