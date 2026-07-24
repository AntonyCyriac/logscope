/**
 * @file error_clusterer_test.cpp
 */

#include <gtest/gtest.h>

#include "analysis_model.hpp"
#include "error_clusterer.hpp"
#include "line_index.hpp"

using scope::analysis::AnalysisModel;
using scope::analysis::DetectedLogLevel;
using scope::analysis::IndexedLine;
using scope::analysis::LineIndex;
using scope::analysis::LogLevelCounts;
using scope::analysis::makeLineIndex;
using scope::analytics::AnalyticsConfig;
using scope::analytics::ErrorClusterer;
using scope::foundation::Path;

TEST(ErrorClustererTest, GroupsSimilarErrorsBySignature)
{
    LineIndex index = makeLineIndex();

    IndexedLine first;
    first.lineNumber = 1U;
    first.level = DetectedLogLevel::Error;
    first.messageExcerpt = "Error code=500";
    EXPECT_TRUE(index.tryAddLine(first));

    IndexedLine second = first;
    second.lineNumber = 2U;
    second.messageExcerpt = "Error code=503";
    EXPECT_TRUE(index.tryAddLine(second));

    LogLevelCounts counts;
    counts.recordError();
    counts.recordError();

    const AnalysisModel model(Path("sample.log"), 2U, counts, scope::analysis::LogFormat::PlainText, std::nullopt,
                              std::nullopt, std::move(index));

    const ErrorClusterer clusterer;
    const auto result = clusterer.analyze(model, AnalyticsConfig{});

    ASSERT_EQ(1U, result.clusters().size());
    EXPECT_EQ(2U, result.clusters()[0].count);
    EXPECT_EQ("Error code=N", result.clusters()[0].signature);
}
