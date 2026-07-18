/**
 * @file log_line_classifier_test.cpp
 * @brief Unit tests for log line classification.
 */

#include <gtest/gtest.h>

#include "log_line_classifier.hpp"

using scope::analysis::DetectedLogLevel;
using scope::analysis::detectLogLevel;

TEST(LogLineClassifierTest, DetectsCommonLevels)
{
    EXPECT_EQ(DetectedLogLevel::Info, detectLogLevel("2026-07-11 10:00:01 INFO Application started"));
    EXPECT_EQ(DetectedLogLevel::Warn, detectLogLevel("2026-07-11 10:00:10 WARNING Request taking too long"));
    EXPECT_EQ(DetectedLogLevel::Error, detectLogLevel("2026-07-11 10:00:06 ERROR Connection refused"));
}

TEST(LogLineClassifierTest, DetectsBlankAndOtherLines)
{
    EXPECT_EQ(DetectedLogLevel::Blank, detectLogLevel("   "));
    EXPECT_EQ(DetectedLogLevel::Other, detectLogLevel("plain unstructured line"));
}
