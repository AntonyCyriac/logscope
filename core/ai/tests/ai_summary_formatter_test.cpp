/**
 * @file ai_summary_formatter_test.cpp
 */

#include <gtest/gtest.h>

#include "ai_summary_formatter.hpp"

using scope::ai::AiEvidence;
using scope::ai::AiSummary;
using scope::ai::formatAiSummary;

TEST(AiSummaryFormatterTest, RendersStructuredSections)
{
    AiSummary summary;
    summary.summary = "2 matching lines.";
    summary.reasoning = "Rule-based summary.";
    summary.confidence = "medium";
    summary.evidence.push_back(AiEvidence{3U, "Connection refused"});
    summary.suggestedActions.push_back("Review evidence lines.");

    const std::string formatted = formatAiSummary(summary);

    EXPECT_NE(std::string::npos, formatted.find("Summary\n2 matching lines."));
    EXPECT_NE(std::string::npos, formatted.find("Reasoning\nRule-based summary."));
    EXPECT_NE(std::string::npos, formatted.find("Evidence\n- Line 3: Connection refused"));
    EXPECT_NE(std::string::npos, formatted.find("Confidence: medium"));
    EXPECT_NE(std::string::npos, formatted.find("Suggested actions\n- Review evidence lines."));
}
