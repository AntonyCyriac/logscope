/**
 * @file investigation_view_test.cpp
 * @brief Unit tests for InvestigationView.
 */

#include <gtest/gtest.h>

#include "investigation.hpp"

using scope::foundation::Path;
using scope::investigation::InvestigationView;

TEST(InvestigationViewTest, ReportsSourceAndLineCount)
{
    const InvestigationView view(Path("sample.log"), 12U);

    EXPECT_EQ("sample.log", view.sourcePath().string());
    EXPECT_EQ(12U, view.totalLines());
    EXPECT_FALSE(view.isEmpty());
    EXPECT_EQ("source=sample.log, lines=12", view.summary());
}

TEST(InvestigationViewTest, DetectsEmptyAnalysis)
{
    const InvestigationView view(Path("empty.log"), 0U);

    EXPECT_TRUE(view.isEmpty());
    EXPECT_EQ("source=empty.log, lines=0", view.summary());
}
