/**
 * @file timeline_analyzer_test.cpp
 */

#include <gtest/gtest.h>

#include "analysis_model.hpp"
#include "field_summary.hpp"
#include "line_index.hpp"
#include "timeline_analyzer.hpp"

using scope::analysis::AnalysisModel;
using scope::analysis::DetectedLogLevel;
using scope::analysis::FieldSummary;
using scope::analysis::IndexedLine;
using scope::analysis::LineIndex;
using scope::analysis::LogLevelCounts;
using scope::analysis::makeLineIndex;
using scope::analytics::AnalyticsConfig;
using scope::analytics::TimelineAnalyzer;
using scope::foundation::Path;
using scope::foundation::Timestamp;

TEST(TimelineAnalyzerTest, BuildsBucketsFromTimestampedLines)
{
    LineIndex index = makeLineIndex();

    FieldSummary fieldSummary;

    IndexedLine first;
    first.lineNumber = 1U;
    first.level = DetectedLogLevel::Error;
    first.timestamp = Timestamp::fromUnixSeconds(1000);
    fieldSummary.recordTimestamp(*first.timestamp);
    EXPECT_TRUE(index.tryAddLine(first));

    IndexedLine second;
    second.lineNumber = 2U;
    second.level = DetectedLogLevel::Info;
    second.timestamp = Timestamp::fromUnixSeconds(1300);
    fieldSummary.recordTimestamp(*second.timestamp);
    EXPECT_TRUE(index.tryAddLine(second));

    LogLevelCounts counts;
    counts.recordError();
    counts.recordInfo();

    AnalyticsConfig config;
    config.bucketSeconds = 200;

    const AnalysisModel model(Path("sample.log"), 2U, counts, scope::analysis::LogFormat::PlainText, std::nullopt,
                              fieldSummary, std::move(index));

    const TimelineAnalyzer analyzer;
    const auto timeline = analyzer.analyze(model, config);

    ASSERT_TRUE(timeline.hasData());
    EXPECT_EQ(200, timeline.bucketSeconds());
    EXPECT_GE(timeline.buckets().size(), 2U);
}
