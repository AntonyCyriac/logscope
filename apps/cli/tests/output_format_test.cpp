/**
 * @file output_format_test.cpp
 * @brief Unit tests for OutputFormat parsing.
 */

#include <gtest/gtest.h>

#include "output_format.hpp"

using scope::cli::OutputFormat;
using scope::cli::outputFormatName;
using scope::cli::parseOutputFormat;

TEST(OutputFormatTest, ParsesSupportedFormats)
{
    ASSERT_TRUE(parseOutputFormat("text"));
    EXPECT_EQ(OutputFormat::Text, *parseOutputFormat("text"));
    EXPECT_EQ(OutputFormat::Text, *parseOutputFormat("TEXT"));

    ASSERT_TRUE(parseOutputFormat("json"));
    EXPECT_EQ(OutputFormat::Json, *parseOutputFormat("json"));
    EXPECT_EQ(OutputFormat::Json, *parseOutputFormat("JSON"));
}

TEST(OutputFormatTest, RejectsUnknownFormat)
{
    EXPECT_FALSE(parseOutputFormat("xml"));
    EXPECT_FALSE(parseOutputFormat(""));
}

TEST(OutputFormatTest, ReturnsCanonicalNames)
{
    EXPECT_EQ("text", outputFormatName(OutputFormat::Text));
    EXPECT_EQ("json", outputFormatName(OutputFormat::Json));
}
