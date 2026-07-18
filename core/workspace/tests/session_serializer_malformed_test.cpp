/**
 * @file session_serializer_malformed_test.cpp
 * @brief Malformed-input tests for SessionSerializer.
 */

#include <gtest/gtest.h>

#include "session_serializer.hpp"

using scope::workspace::SessionSerializer;

TEST(SessionSerializerMalformedTest, RejectsEmptyContent)
{
    const auto result = SessionSerializer::deserialize("");

    EXPECT_FALSE(result.hasValue());
}

TEST(SessionSerializerMalformedTest, RejectsMissingSourcePath)
{
    const auto result = SessionSerializer::deserialize("session.version=1.0\nanalysis.totalLines=10\n");

    EXPECT_FALSE(result.hasValue());
}

TEST(SessionSerializerMalformedTest, RejectsInvalidNumericFields)
{
    const std::string content =
        "source.path=sample.log\n"
        "analysis.totalLines=not-a-number\n";

    const auto result = SessionSerializer::deserialize(content);

    EXPECT_FALSE(result.hasValue());
}

TEST(SessionSerializerMalformedTest, AcceptsGarbageLinesWithoutCrashing)
{
    const std::string content =
        "!!!garbage!!!\n"
        "source.path=sample.log\n"
        "analysis.totalLines=5\n"
        "analysis.infoLines=1\n"
        "analysis.warningLines=1\n"
        "analysis.errorLines=1\n"
        "analysis.otherLines=1\n"
        "analysis.blankLines=1\n"
        "filter.minLines=0\n"
        "filter.maxLines=\n"
        "filter.minErrors=0\n"
        "filter.minWarnings=0\n"
        "filter.searchQuery=\n"
        "report.format=text\n"
        "report.sections=all\n"
        "config.path=\n";

    const auto result = SessionSerializer::deserialize(content);

    ASSERT_TRUE(result.hasValue());
    EXPECT_EQ(5U, result->analysisModel().totalLines());
}

TEST(SessionSerializerMalformedTest, HandlesVeryLongLines)
{
    std::string content = "source.path=";
    content.append(10000U, 'x');
    content += "\nanalysis.totalLines=1\n";

    const auto result = SessionSerializer::deserialize(content);

    ASSERT_TRUE(result.hasValue());
}
