/**
 * @file investigation_engine_test.cpp
 * @brief Unit tests for InvestigationEngine.
 */

#include <gtest/gtest.h>

#include "analysis.hpp"
#include "investigation.hpp"

using scope::analysis::AnalysisModel;
using scope::foundation::Path;
using scope::investigation::InvestigationEngine;
using scope::investigation::InvestigationView;
using scope::investigation::LineCountFilter;

TEST(InvestigationEngineTest, InspectsAnalysisModel)
{
    const AnalysisModel model(Path("sample.log"), 8U);

    InvestigationEngine engine;

    const InvestigationView view = engine.inspect(model);

    EXPECT_EQ("sample.log", view.sourcePath().string());
    EXPECT_EQ(8U, view.totalLines());
    EXPECT_EQ("source=sample.log, lines=8", view.summary());
}

TEST(InvestigationEngineTest, AppliesLineCountFilter)
{
    const AnalysisModel model(Path("sample.log"), 8U);

    InvestigationEngine engine;

    EXPECT_TRUE(engine.matches(model, LineCountFilter::nonEmpty()));
    EXPECT_FALSE(engine.matches(model, LineCountFilter::any().withMinimum(9U)));
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
