/**
 * @file ai_context_builder_test.cpp
 */

#include <gtest/gtest.h>

#include "ai_context_builder.hpp"
#include "analysis.hpp"
#include "investigation.hpp"
#include "line_index.hpp"

using scope::ai::AiConfig;
using scope::ai::buildInvestigationContext;
using scope::analysis::AnalysisModel;
using scope::analysis::DetectedLogLevel;
using scope::analysis::IndexedLine;
using scope::analysis::LineIndex;
using scope::analysis::LogLevelCounts;
using scope::analysis::makeLineIndex;
using scope::foundation::Path;
using scope::investigation::InvestigationCriteria;
using scope::investigation::InvestigationEngine;
using scope::investigation::InvestigationResult;
using scope::investigation::InvestigationView;

namespace
{

AnalysisModel createIndexedModel()
{
    LineIndex lineIndex = makeLineIndex();

    IndexedLine firstLine;
    firstLine.lineNumber = 3U;
    firstLine.level = DetectedLogLevel::Error;
    firstLine.messageExcerpt = "Connection refused";
    firstLine.contentExcerpt = "2026-07-11 ERROR Connection refused";
    EXPECT_TRUE(lineIndex.tryAddLine(firstLine));

    IndexedLine secondLine = firstLine;
    secondLine.lineNumber = 4U;
    EXPECT_TRUE(lineIndex.tryAddLine(secondLine));

    LogLevelCounts levelCounts;
    levelCounts.recordError();
    levelCounts.recordError();

    return AnalysisModel(Path("sample.log"), 4U, levelCounts, scope::analysis::LogFormat::PlainText, std::nullopt,
                         std::nullopt, std::move(lineIndex));
}

} // namespace

TEST(AiContextBuilderTest, BoundsSampleLinesToMaxContextLines)
{
    AiConfig config;
    config.maxContextLines = 1U;

    InvestigationEngine engine;
    InvestigationCriteria criteria;
    criteria.contentSearch = "refused";

    const AnalysisModel model = createIndexedModel();
    const InvestigationView view = engine.inspect(model);
    const auto result = engine.investigate(model, criteria);

    const auto context = buildInvestigationContext(config, view, result);

    EXPECT_EQ(2U, context.matchCount);
    EXPECT_EQ(1U, context.sampleLines.size());
    EXPECT_EQ(3U, context.sampleLines[0].lineNumber);
    EXPECT_EQ("Connection refused", context.sampleLines[0].text);
    EXPECT_EQ(1U, context.repeatedErrorPatternCount);
    EXPECT_EQ("Connection refused", context.topRepeatedErrorKey);
    EXPECT_EQ(2U, context.topRepeatedErrorCount);
    EXPECT_NE(std::string::npos, context.sourceSummary.find("sample.log"));
}

TEST(AiContextBuilderTest, PreservesEmptyMatchContext)
{
    AiConfig config;

    const AnalysisModel model(Path("empty.log"), 0U);
    const InvestigationEngine engine;
    const InvestigationView view = engine.inspect(model);

    InvestigationResult result;
    result.indexedLineCount = 0U;

    const auto context = buildInvestigationContext(config, view, result);

    EXPECT_EQ(0U, context.matchCount);
    EXPECT_TRUE(context.sampleLines.empty());
}
