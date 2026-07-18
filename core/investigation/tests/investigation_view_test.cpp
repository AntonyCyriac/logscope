/**
 * @file investigation_view_test.cpp
 * @brief Unit tests for InvestigationView.
 */

#include <gtest/gtest.h>

#include "analysis.hpp"
#include "investigation.hpp"

using scope::analysis::AnalysisModel;
using scope::analysis::LogLevelCounts;
using scope::foundation::Path;
using scope::investigation::InvestigationView;

namespace
{

AnalysisModel createSampleModel()
{
    LogLevelCounts levelCounts;
    levelCounts.recordInfo();
    levelCounts.recordInfo();
    levelCounts.recordWarn();
    levelCounts.recordError();
    levelCounts.recordError();

    return AnalysisModel(Path("sample.log"), 5U, levelCounts);
}

} // namespace

TEST(InvestigationViewTest, ReportsSourceAndLineCount)
{
    const InvestigationView view(createSampleModel());

    EXPECT_EQ("sample.log", view.sourcePath().string());
    EXPECT_EQ(5U, view.totalLines());
    EXPECT_FALSE(view.isEmpty());
    EXPECT_NE(std::string::npos, view.summary().find("errors=2"));
    EXPECT_NE(std::string::npos, view.summary().find("warnings=1"));
    EXPECT_NE(std::string::npos, view.summary().find("info=2"));
}

TEST(InvestigationViewTest, DetectsEmptyAnalysis)
{
    const InvestigationView view(AnalysisModel(Path("empty.log"), 0U));

    EXPECT_TRUE(view.isEmpty());
    EXPECT_NE(std::string::npos, view.summary().find("lines=0"));
}
