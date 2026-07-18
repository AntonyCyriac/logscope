/**
 * @file investigation_engine_test.cpp
 * @brief Unit tests for InvestigationEngine.
 */

#include <gtest/gtest.h>

#include "analysis.hpp"
#include "investigation.hpp"

using scope::analysis::AnalysisModel;
using scope::analysis::LogLevelCounts;
using scope::foundation::Path;
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
