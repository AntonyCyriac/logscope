/**
 * @file correlation_analyzer_test.cpp
 */

#include <gtest/gtest.h>

#include "analysis_model.hpp"
#include "correlation_analyzer.hpp"
#include "line_index.hpp"

using scope::analysis::AnalysisModel;
using scope::analysis::DetectedLogLevel;
using scope::analysis::IndexedLine;
using scope::analysis::LineIndex;
using scope::analysis::LogLevelCounts;
using scope::analysis::makeLineIndex;
using scope::analytics::CorrelationAnalyzer;
using scope::foundation::Path;

TEST(CorrelationAnalyzerTest, FindsRepeatedErrorsAndSharedIds)
{
    LineIndex index = makeLineIndex();

    IndexedLine line;
    line.lineNumber = 1U;
    line.level = DetectedLogLevel::Error;
    line.messageExcerpt = "Connection refused";
    line.correlationId = "trace-abc";
    index.tryAddLine(line);

    line.lineNumber = 2U;
    index.tryAddLine(line);

    LogLevelCounts counts;
    counts.recordError();
    counts.recordError();

    const AnalysisModel model(Path("sample.log"), 2U, counts, scope::analysis::LogFormat::PlainText, std::nullopt,
                              std::nullopt, std::move(index));

    const CorrelationAnalyzer analyzer;
    const auto result = analyzer.analyze(model);

    ASSERT_EQ(1U, result.repeatedErrors().size());
    EXPECT_EQ(2U, result.repeatedErrors()[0].count);
    ASSERT_EQ(1U, result.sharedCorrelationIds().size());
    EXPECT_EQ("trace-abc", result.sharedCorrelationIds()[0].key);
}
