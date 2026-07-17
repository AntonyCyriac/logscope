/**
 * @file result_test.cpp
 * @brief Unit tests for Result<T>.
 */

#include <string>

#include <gtest/gtest.h>

#include "foundation.hpp"

using scope::foundation::Error;
using scope::foundation::ErrorCode;
using scope::foundation::Result;

TEST(ResultTest, ConstructFromConstValue)
{
    const int value = 42;

    Result<int> result(value);

    EXPECT_TRUE(result.hasValue());
    EXPECT_FALSE(result.hasError());
    EXPECT_TRUE(result);

    EXPECT_EQ(42, result.value());
}

TEST(ResultTest, ConstructFromMovedValue)
{
    std::string value = "LogScope";

    Result<std::string> result(std::move(value));

    EXPECT_TRUE(result.hasValue());
    EXPECT_FALSE(result.hasError());

    EXPECT_EQ("LogScope", result.value());
}

TEST(ResultTest, ConstructFromConstError)
{
    const Error error(ErrorCode::InvalidArgument, "Invalid value");

    Result<int> result(error);

    EXPECT_FALSE(result.hasValue());
    EXPECT_TRUE(result.hasError());
    EXPECT_FALSE(result);

    EXPECT_EQ(ErrorCode::InvalidArgument, result.error().code());

    EXPECT_EQ("Invalid value", result.error().message());
}

TEST(ResultTest, ConstructFromMovedError)
{
    Error error(ErrorCode::ParseError, "Parse failed");

    Result<int> result(std::move(error));

    EXPECT_TRUE(result.hasError());

    EXPECT_EQ(ErrorCode::ParseError, result.error().code());

    EXPECT_EQ("Parse failed", result.error().message());
}

TEST(ResultTest, MutableValue)
{
    Result<int> result(10);

    result.value() = 25;

    EXPECT_EQ(25, result.value());
}

TEST(ResultTest, ConstDereference)
{
    const Result<int> result(100);

    EXPECT_EQ(100, *result);
}

TEST(ResultTest, MutableDereference)
{
    Result<int> result(100);

    *result = 200;

    EXPECT_EQ(200, result.value());
}

TEST(ResultTest, ConstArrow)
{
    const Result<std::string> result("LogScope");

    EXPECT_EQ(8U, result->size());
}

TEST(ResultTest, MutableArrow)
{
    Result<std::string> result("LogScope");

    result->append(" Platform");

    EXPECT_EQ("LogScope Platform", result.value());
}

TEST(ResultTest, ValueThrowsWhenResultContainsError)
{
    Result<int> result(Error(ErrorCode::Unknown, "Failure"));

    EXPECT_THROW(result.value(), std::logic_error);
}

TEST(ResultTest, ErrorThrowsWhenResultContainsValue)
{
    Result<int> result(42);

    EXPECT_THROW(result.error(), std::logic_error);
}

TEST(ResultTest, BoolOperator)
{
    Result<int> success(1);

    Result<int> failure(Error(ErrorCode::Unknown, "Failure"));

    EXPECT_TRUE(success);

    EXPECT_FALSE(failure);
}
