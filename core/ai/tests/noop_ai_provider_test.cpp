/**
 * @file noop_ai_provider_test.cpp
 */

#include <gtest/gtest.h>

#include "noop_ai_provider.hpp"

using scope::ai::AiAnalyticsContext;
using scope::ai::AiInvestigationContext;
using scope::ai::NoOpAiProvider;

TEST(NoOpAiProviderTest, MapsErrorsKeywordToFilterDsl)
{
    const NoOpAiProvider provider;

    const auto result = provider.translateNlToFilter("show errors");

    ASSERT_TRUE(result);
    EXPECT_EQ(*result, "level == ERROR");
}

TEST(NoOpAiProviderTest, RejectsUnmappedNaturalLanguage)
{
    const NoOpAiProvider provider;

    const auto result = provider.translateNlToFilter("show everything unusual");

    EXPECT_FALSE(result);
}

TEST(NoOpAiProviderTest, SummarizeUsesInvestigationContext)
{
    const NoOpAiProvider provider;

    AiInvestigationContext context;
    context.sourceSummary = "source=sample.log, lines=4, errors=2, warnings=0, info=0";
    context.matchCount = 2U;
    context.indexedLineCount = 4U;
    context.searchQuerySummary = "content:refused";
    context.repeatedErrorPatternCount = 1U;
    context.topRepeatedErrorKey = "Connection refused";
    context.topRepeatedErrorCount = 2U;

    const auto result = provider.summarize(context);

    ASSERT_TRUE(result);
    EXPECT_NE(std::string::npos, result->summary.find("2 matching lines"));
    EXPECT_NE(std::string::npos, result->summary.find("content:refused"));
    EXPECT_NE(std::string::npos, result->reasoning.find("Connection refused"));
    EXPECT_EQ("medium", result->confidence);
    EXPECT_FALSE(result->suggestedActions.empty());
}

TEST(NoOpAiProviderTest, SummarizeHandlesNoMatches)
{
    const NoOpAiProvider provider;

    AiInvestigationContext context;
    context.matchCount = 0U;
    context.searchQuerySummary = "level == ERROR";

    const auto result = provider.summarize(context);

    ASSERT_TRUE(result);
    EXPECT_NE(std::string::npos, result->summary.find("No lines matched"));
    EXPECT_EQ("low", result->confidence);
}

TEST(NoOpAiProviderTest, SuggestAnomaliesUsesAnalyticsSignals)
{
    const NoOpAiProvider provider;

    AiAnalyticsContext context;
    context.hasSpike = true;
    context.spikeVerdict = "Spike in ERROR rate";
    context.clusterCount = 1U;
    context.topClusterMessage = "connection reset";
    context.topClusterCount = 3U;

    const auto result = provider.suggestAnomalies(context);

    ASSERT_TRUE(result);
    ASSERT_EQ(result->size(), 2U);
    EXPECT_NE(std::string::npos, result->at(1).message.find("connection reset"));
    EXPECT_NE(std::string::npos, result->at(1).message.find("3 occurrences"));
}

TEST(NoOpAiProviderTest, SuggestAnomaliesReportsNoSignals)
{
    const NoOpAiProvider provider;

    const auto result = provider.suggestAnomalies(AiAnalyticsContext{});

    ASSERT_TRUE(result);
    ASSERT_EQ(1U, result->size());
    EXPECT_EQ("info", result->front().severity);
    EXPECT_NE(std::string::npos, result->front().message.find("No anomaly signals"));
}

TEST(NoOpAiProviderTest, SuggestAnomaliesUsesCorrelationWhenNoCluster)
{
    const NoOpAiProvider provider;

    AiAnalyticsContext context;
    context.repeatedErrorPatternCount = 1U;
    context.topRepeatedErrorKey = "timeout";
    context.topRepeatedErrorCount = 4U;

    const auto result = provider.suggestAnomalies(context);

    ASSERT_TRUE(result);
    ASSERT_EQ(1U, result->size());
    EXPECT_NE(std::string::npos, result->front().message.find("Repeated error pattern: timeout"));
}
