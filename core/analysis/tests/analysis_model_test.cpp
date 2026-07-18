/**
 * @file analysis_model_test.cpp
 * @brief Unit tests for AnalysisModel.
 */

#include <gtest/gtest.h>

#include "analysis.hpp"

using scope::analysis::AnalysisModel;
using scope::analysis::LogLevelCounts;
using scope::foundation::Path;

TEST(AnalysisModelTest, StoresSourcePathAndLineCount)
{
    const AnalysisModel model(Path("sample.log"), 42U);

    EXPECT_EQ("sample.log", model.sourcePath().string());
    EXPECT_EQ(42U, model.totalLines());
    EXPECT_EQ(0U, model.levelCounts().errorLines());
}

TEST(AnalysisModelTest, StoresLevelCounts)
{
    LogLevelCounts levelCounts;
    levelCounts.recordError();
    levelCounts.recordError();
    levelCounts.recordWarn();
    levelCounts.recordInfo();

    const AnalysisModel model(Path("sample.log"), 4U, levelCounts);

    EXPECT_EQ(2U, model.levelCounts().errorLines());
    EXPECT_EQ(1U, model.levelCounts().warnLines());
    EXPECT_EQ(1U, model.levelCounts().infoLines());
}

TEST(AnalysisModelTest, EmptySourceHasZeroLines)
{
    const AnalysisModel model(Path("empty.log"), 0U);

    EXPECT_EQ(0U, model.totalLines());
}
