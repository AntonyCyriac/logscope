/**
 * @file analytics_engine_test.cpp
 */

#include <gtest/gtest.h>

#include "analytics_engine.hpp"
#include "analysis_model.hpp"
#include "line_index.hpp"

using scope::analysis::AnalysisModel;
using scope::analysis::DetectedLogLevel;
using scope::analysis::IndexedLine;
using scope::analysis::LineIndex;
using scope::analysis::LogLevelCounts;
using scope::analysis::makeLineIndex;
using scope::analytics::AnalyticsEngine;
using scope::foundation::Path;

TEST(AnalyticsEngineTest, ProducesCombinedAnalyticsResult)
{
    LineIndex index = makeLineIndex();

    IndexedLine line;
    line.lineNumber = 1U;
    line.level = DetectedLogLevel::Error;
    line.messageExcerpt = "Connection refused";
    line.correlationId = "trace-abc";
    EXPECT_TRUE(index.tryAddLine(line));

    line.lineNumber = 2U;
    EXPECT_TRUE(index.tryAddLine(line));

    LogLevelCounts counts;
    counts.recordError();
    counts.recordError();

    const AnalysisModel model(Path("sample.log"), 2U, counts, scope::analysis::LogFormat::PlainText, std::nullopt,
                              std::nullopt, std::move(index));

    const AnalyticsEngine engine;
    const auto result = engine.analyze(model);

    EXPECT_EQ(2U, result.indexedLineCount());
    EXPECT_FALSE(result.frequency().topMessages().empty());
    EXPECT_FALSE(result.clusters().clusters().empty());
    EXPECT_FALSE(result.correlations().repeatedErrors().empty());
}
