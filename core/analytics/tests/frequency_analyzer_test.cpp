/**
 * @file frequency_analyzer_test.cpp
 */

#include <gtest/gtest.h>

#include "analysis_model.hpp"
#include "frequency_analyzer.hpp"
#include "line_index.hpp"

using scope::analysis::AnalysisModel;
using scope::analysis::DetectedLogLevel;
using scope::analysis::IndexedLine;
using scope::analysis::LineIndex;
using scope::analysis::LogLevelCounts;
using scope::analysis::makeLineIndex;
using scope::analytics::AnalyticsConfig;
using scope::analytics::FrequencyAnalyzer;
using scope::foundation::Path;

namespace
{

AnalysisModel createModel()
{
    LineIndex index = makeLineIndex();

    IndexedLine line;
    line.lineNumber = 1U;
    line.level = DetectedLogLevel::Error;
    line.messageExcerpt = "Connection refused";
    line.correlationId = "trace-1";
    index.tryAddLine(line);

    line.lineNumber = 2U;
    index.tryAddLine(line);

    LogLevelCounts counts;
    counts.recordError();
    counts.recordError();

    return AnalysisModel(Path("sample.log"), 2U, counts, scope::analysis::LogFormat::PlainText, std::nullopt,
                         std::nullopt, std::move(index));
}

} // namespace

TEST(FrequencyAnalyzerTest, RanksTopMessagesAndCorrelationIds)
{
    const FrequencyAnalyzer analyzer;
    const auto result = analyzer.analyze(createModel(), AnalyticsConfig{});

    ASSERT_EQ(1U, result.topMessages().size());
    EXPECT_EQ("Connection refused", result.topMessages()[0].key);
    EXPECT_EQ(2U, result.topMessages()[0].count);
    ASSERT_EQ(1U, result.topCorrelationIds().size());
    EXPECT_EQ("trace-1", result.topCorrelationIds()[0].key);
}
