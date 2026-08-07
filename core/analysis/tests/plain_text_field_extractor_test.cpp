/**
 * @file plain_text_field_extractor_test.cpp
 * @brief Unit tests for plain-text field extraction.
 */

#include <gtest/gtest.h>

#include "plain_text_field_extractor.hpp"

using scope::analysis::PlainTextFieldExtractor;
using scope::analysis::parseLogTimestamp;

TEST(PlainTextFieldExtractorTest, ExtractsSpaceSeparatedTimestampAndMessage)
{
    const auto fields =
        PlainTextFieldExtractor::extract("2026-07-11 10:00:01 INFO Application started");

    ASSERT_TRUE(fields.timestamp.has_value());
    EXPECT_EQ("Application started", fields.messageExcerpt);
}

TEST(PlainTextFieldExtractorTest, ExtractsIsoTimestamp)
{
    const auto fields = PlainTextFieldExtractor::extract("2026-07-11T10:00:01 ERROR Connection refused");

    ASSERT_TRUE(fields.timestamp.has_value());
    EXPECT_EQ("Connection refused", fields.messageExcerpt);
}

TEST(PlainTextFieldExtractorTest, ParsesZuluTimestampSuffix)
{
    const auto timestamp = parseLogTimestamp("2026-07-21T10:00:01Z");

    ASSERT_TRUE(timestamp.hasValue());
}

TEST(PlainTextFieldExtractorTest, ExtractsFractionalSecondTimestampWithoutLeakingIntoMessage)
{
    const auto fields = PlainTextFieldExtractor::extract(
        "2026-07-28T09:15:01.101Z FE01 INFO ManagerA started");

    ASSERT_TRUE(fields.timestamp.has_value());
    EXPECT_EQ("2026-07-28T09:15:01.101Z", fields.timestamp->toString());
    EXPECT_EQ("FE01 INFO ManagerA started", fields.messageExcerpt);
    EXPECT_NE('.', fields.messageExcerpt.empty() ? ' ' : fields.messageExcerpt.front());
}

TEST(PlainTextFieldExtractorTest, DistinguishesSubSecondTimestamps)
{
    const auto first =
        PlainTextFieldExtractor::extract("2026-07-28T09:15:01.101Z FE01 INFO ManagerA started");
    const auto second =
        PlainTextFieldExtractor::extract("2026-07-28T09:15:01.842Z FE01 INFO ManagerB finished");

    ASSERT_TRUE(first.timestamp.has_value());
    ASSERT_TRUE(second.timestamp.has_value());
    EXPECT_NE(first.timestamp->toString(), second.timestamp->toString());
    EXPECT_LT(*first.timestamp, *second.timestamp);
}
