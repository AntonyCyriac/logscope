/**
 * @file ai_analytics_context_builder_test.cpp
 */

#include <gtest/gtest.h>

#include "ai_analytics_context_builder.hpp"
#include "analytics_engine.hpp"
#include "analysis_model.hpp"
#include "line_index.hpp"
#include "trend_result.hpp"

using scope::ai::buildAnalyticsContext;
using scope::analysis::AnalysisModel;
using scope::analysis::DetectedLogLevel;
using scope::analysis::IndexedLine;
using scope::analysis::LineIndex;
using scope::analysis::LogLevelCounts;
using scope::analysis::makeLineIndex;
using scope::analytics::AnalyticsEngine;
using scope::analytics::AnalyticsResult;
using scope::analytics::TrendResult;
using scope::foundation::Path;

TEST(AiAnalyticsContextBuilderTest, MapsClustersAndCorrelationsFromEngineOutput)
{
    LineIndex index = makeLineIndex();

    IndexedLine line;
    line.lineNumber = 1U;
    line.level = DetectedLogLevel::Error;
    line.messageExcerpt = "Connection refused";
    line.correlationId = "trace-abc";
    ASSERT_TRUE(index.tryAddLine(line));

    line.lineNumber = 2U;
    ASSERT_TRUE(index.tryAddLine(line));

    LogLevelCounts counts;
    counts.recordError();
    counts.recordError();

    const AnalysisModel model(Path("sample.log"), 2U, counts, scope::analysis::LogFormat::PlainText, std::nullopt,
                              std::nullopt, std::move(index));

    const AnalyticsEngine engine;
    const auto analytics = engine.analyze(model);
    const auto context = buildAnalyticsContext(analytics);

    EXPECT_GT(context.clusterCount, 0U);
    EXPECT_EQ("Connection refused", context.topClusterMessage);
    EXPECT_EQ(2U, context.topClusterCount);
    EXPECT_GT(context.repeatedErrorPatternCount, 0U);
    EXPECT_EQ("Connection refused", context.topRepeatedErrorKey);
    EXPECT_EQ(2U, context.topRepeatedErrorCount);
}

TEST(AiAnalyticsContextBuilderTest, MapsTrendSpikeSignals)
{
    AnalyticsResult analytics;

    TrendResult trends;
    trends.setHasSpike(true);
    trends.setVerdict("Spike detected: error burst above timeline average");
    analytics.setTrends(std::move(trends));

    const auto context = buildAnalyticsContext(analytics);

    EXPECT_TRUE(context.hasSpike);
    EXPECT_NE(std::string::npos, context.spikeVerdict.find("Spike detected"));
}

TEST(AiAnalyticsContextBuilderTest, PreservesEmptyAnalyticsContext)
{
    const AnalyticsResult analytics;
    const auto context = buildAnalyticsContext(analytics);

    EXPECT_FALSE(context.hasSpike);
    EXPECT_EQ(0U, context.clusterCount);
    EXPECT_EQ(0U, context.repeatedErrorPatternCount);
}
