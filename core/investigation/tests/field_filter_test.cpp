/**
 * @file field_filter_test.cpp
 * @brief Unit tests for FieldFilter.
 */

#include <gtest/gtest.h>

#include "field_filter.hpp"
#include "line_index.hpp"
#include "log_line_classifier.hpp"

using scope::analysis::DetectedLogLevel;
using scope::analysis::IndexedLine;
using scope::investigation::FieldFilter;

TEST(FieldFilterTest, MatchesLevelMessageAndJsonKey)
{
    const FieldFilter filter = FieldFilter::any()
                                   .withLevel(DetectedLogLevel::Error)
                                   .withMessageContains("refused")
                                   .withRequiredJsonKey("timestamp");

    IndexedLine line;
    line.level = DetectedLogLevel::Error;
    line.messageExcerpt = "Connection refused";
    line.topLevelKeys = {"timestamp", "message"};

    EXPECT_TRUE(filter.matches(line));
}

TEST(FieldFilterTest, RejectsMissingJsonKey)
{
    const FieldFilter filter = FieldFilter::any().withRequiredJsonKey("timestamp");

    IndexedLine line;
    line.topLevelKeys = {"message"};

    EXPECT_FALSE(filter.matches(line));
}
