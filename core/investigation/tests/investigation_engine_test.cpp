/**
 * @file investigation_engine_test.cpp
 * @brief Unit tests for InvestigationEngine.
 */

#include <gtest/gtest.h>

#include "analysis.hpp"
#include "investigation.hpp"
#include "line_index.hpp"

using scope::analysis::DetectedLogLevel;
using scope::analysis::IndexedLine;
using scope::analysis::LineIndex;
using scope::analysis::makeLineIndex;

using scope::analysis::AnalysisModel;
using scope::analysis::LogLevelCounts;
using scope::foundation::Path;
using scope::investigation::InvestigationCriteria;
using scope::investigation::InvestigationEngine;
using scope::investigation::InvestigationView;
using scope::investigation::LineCountFilter;
using scope::investigation::LogLevelFilter;

namespace
{

AnalysisModel createSampleModel()
{
    LogLevelCounts levelCounts;
    levelCounts.recordInfo();
    levelCounts.recordWarn();
    levelCounts.recordError();
    levelCounts.recordError();

    return AnalysisModel(Path("sample.log"), 4U, levelCounts);
}

} // namespace

TEST(InvestigationEngineTest, InspectsAnalysisModel)
{
    InvestigationEngine engine;

    const InvestigationView view = engine.inspect(createSampleModel());

    EXPECT_EQ("sample.log", view.sourcePath().string());
    EXPECT_EQ(4U, view.totalLines());
    EXPECT_NE(std::string::npos, view.summary().find("errors=2"));
}

TEST(InvestigationEngineTest, AppliesLineCountFilter)
{
    InvestigationEngine engine;

    const AnalysisModel model(Path("sample.log"), 8U);

    EXPECT_TRUE(engine.matches(model, LineCountFilter::nonEmpty()));
    EXPECT_FALSE(engine.matches(model, LineCountFilter::any().withMinimum(9U)));
}

TEST(InvestigationEngineTest, AppliesLogLevelFilter)
{
    InvestigationEngine engine;

    const AnalysisModel model = createSampleModel();

    EXPECT_TRUE(engine.matches(model, LogLevelFilter::any().withMinimumErrors(2U)));
    EXPECT_FALSE(engine.matches(model, LogLevelFilter::any().withMinimumErrors(3U)));
    EXPECT_TRUE(engine.matches(model, LogLevelFilter::any().withMinimumWarnings(1U)));
}

TEST(InvestigationEngineTest, SearchesSourcePath)
{
    const AnalysisModel model(Path("logs/app/sample.log"), 4U);

    InvestigationEngine engine;

    EXPECT_TRUE(engine.searchSource(model, "sample"));
    EXPECT_TRUE(engine.searchSource(model, "LOGS"));
    EXPECT_FALSE(engine.searchSource(model, "missing"));
    EXPECT_TRUE(engine.searchSource(model, " "));
}

AnalysisModel createIndexedModel()
{
    LineIndex lineIndex = makeLineIndex();

    IndexedLine firstLine;
    firstLine.lineNumber = 3U;
    firstLine.level = DetectedLogLevel::Error;
    firstLine.messageExcerpt = "Connection refused";
    firstLine.contentExcerpt = "2026-07-11 ERROR Connection refused";
    firstLine.correlationId = "trace-abc";
    EXPECT_TRUE(lineIndex.tryAddLine(firstLine));

    IndexedLine secondLine = firstLine;
    secondLine.lineNumber = 4U;
    EXPECT_TRUE(lineIndex.tryAddLine(secondLine));

    IndexedLine thirdLine;
    thirdLine.lineNumber = 5U;
    thirdLine.level = DetectedLogLevel::Info;
    thirdLine.messageExcerpt = "Retrying request";
    thirdLine.contentExcerpt = "2026-07-11 INFO Retrying request";
    EXPECT_TRUE(lineIndex.tryAddLine(thirdLine));

    LogLevelCounts levelCounts;
    levelCounts.recordError();
    levelCounts.recordError();
    levelCounts.recordInfo();

    return AnalysisModel(Path("sample.log"), 5U, levelCounts, scope::analysis::LogFormat::PlainText, std::nullopt,
                         std::nullopt, std::move(lineIndex));
}

TEST(InvestigationEngineTest, SearchContentFindsIndexedLines)
{
    InvestigationEngine engine;
    InvestigationCriteria criteria;
    criteria.contentSearch = "refused";

    const auto result = engine.investigate(createIndexedModel(), criteria);

    EXPECT_EQ(2U, result.matchingLines.size());
}

TEST(InvestigationEngineTest, FindsRepeatedErrorsAndSharedCorrelationIds)
{
    InvestigationEngine engine;

    const auto correlations = engine.findCorrelations(createIndexedModel());

    ASSERT_EQ(1U, correlations.repeatedErrors().size());
    EXPECT_EQ("Connection refused", correlations.repeatedErrors()[0].key);
    EXPECT_EQ(2U, correlations.repeatedErrors()[0].count);
    ASSERT_EQ(1U, correlations.sharedCorrelationIds().size());
    EXPECT_EQ("trace-abc", correlations.sharedCorrelationIds()[0].key);
}
