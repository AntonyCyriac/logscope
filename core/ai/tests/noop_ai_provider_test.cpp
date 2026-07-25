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
    context.sourceSummary = "2 ERROR lines in sample.log";
    context.matchCount = 2U;

    const auto result = provider.summarize(context);

    ASSERT_TRUE(result);
    EXPECT_EQ(result->summary, "2 ERROR lines in sample.log");
    EXPECT_FALSE(result->suggestedActions.empty());
}

TEST(NoOpAiProviderTest, SuggestAnomaliesUsesAnalyticsSignals)
{
    const NoOpAiProvider provider;

    AiAnalyticsContext context;
    context.hasSpike = true;
    context.spikeVerdict = "Spike in ERROR rate";
    context.clusterCount = 1U;
    context.topClusterMessage = "connection reset";

    const auto result = provider.suggestAnomalies(context);

    ASSERT_TRUE(result);
    ASSERT_EQ(result->size(), 2U);
}
