/**
 * @file ai_anomaly_hint_formatter_test.cpp
 */

#include <gtest/gtest.h>

#include "ai_anomaly_hint_formatter.hpp"

using scope::ai::AiAnomalyHint;
using scope::ai::formatAiAnomalyHints;

TEST(AiAnomalyHintFormatterTest, RendersStructuredHints)
{
    std::vector<AiAnomalyHint> hints;
    hints.push_back(AiAnomalyHint{"Spike detected: error burst above timeline average", "medium"});
    hints.push_back(AiAnomalyHint{"Repeated error cluster: connection reset (3 occurrences)", "low"});

    const std::string formatted = formatAiAnomalyHints(hints);

    EXPECT_NE(std::string::npos, formatted.find("Anomaly hints\n"));
    EXPECT_NE(std::string::npos, formatted.find("- [medium] Spike detected"));
    EXPECT_NE(std::string::npos, formatted.find("- [low] Repeated error cluster"));
}

TEST(AiAnomalyHintFormatterTest, ReturnsEmptyStringForNoHints)
{
    EXPECT_TRUE(formatAiAnomalyHints({}).empty());
}
