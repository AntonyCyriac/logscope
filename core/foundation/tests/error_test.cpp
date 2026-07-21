/**
 * @file error_test.cpp
 * @brief Unit tests for the Foundation Error type.
 */

#include <gtest/gtest.h>

#include "foundation.hpp"

using scope::foundation::Error;
using scope::foundation::ErrorCode;

TEST(ErrorTest, DefaultConstruction)
{
    Error error;

    EXPECT_FALSE(error.hasError());
    EXPECT_EQ(ErrorCode::None, error.code());
    EXPECT_TRUE(error.message().empty());
}

TEST(ErrorTest, Construction)
{
    Error error(ErrorCode::InvalidArgument, "Invalid filename");

    EXPECT_TRUE(error.hasError());

    EXPECT_EQ(ErrorCode::InvalidArgument, error.code());

    EXPECT_EQ("Invalid filename", error.message());
}

TEST(ErrorTest, FileNotFound)
{
    Error error(ErrorCode::FileNotFound, "sample.log");

    EXPECT_TRUE(error.hasError());

    EXPECT_EQ(ErrorCode::FileNotFound, error.code());

    EXPECT_EQ("sample.log", error.message());
}

TEST(ErrorTest, CopyPreservesState)
{
    const Error original(ErrorCode::ParseError, "Malformed input");

    const Error copy(original);

    EXPECT_TRUE(copy.hasError());
    EXPECT_EQ(ErrorCode::ParseError, copy.code());
    EXPECT_EQ("Malformed input", copy.message());
}
