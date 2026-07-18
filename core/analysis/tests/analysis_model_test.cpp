/**
 * @file analysis_model_test.cpp
 * @brief Unit tests for AnalysisModel.
 */

#include <gtest/gtest.h>

#include "analysis.hpp"

using scope::analysis::AnalysisModel;
using scope::foundation::Path;

TEST(AnalysisModelTest, StoresSourcePathAndLineCount)
{
    const AnalysisModel model(Path("sample.log"), 42U);

    EXPECT_EQ("sample.log", model.sourcePath().string());
    EXPECT_EQ(42U, model.totalLines());
}

TEST(AnalysisModelTest, EmptySourceHasZeroLines)
{
    const AnalysisModel model(Path("empty.log"), 0U);

    EXPECT_EQ(0U, model.totalLines());
}
