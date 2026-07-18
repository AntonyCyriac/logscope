/**
 * @file line_count_filter_test.cpp
 * @brief Unit tests for LineCountFilter.
 */

#include <gtest/gtest.h>

#include "analysis.hpp"
#include "investigation.hpp"

using scope::analysis::AnalysisModel;
using scope::foundation::Path;
using scope::investigation::LineCountFilter;

TEST(LineCountFilterTest, AnyMatchesAllLineCounts)
{
    const AnalysisModel emptyModel(Path("empty.log"), 0U);
    const AnalysisModel populatedModel(Path("sample.log"), 8U);

    const LineCountFilter filter = LineCountFilter::any();

    EXPECT_TRUE(filter.matches(emptyModel));
    EXPECT_TRUE(filter.matches(populatedModel));
}

TEST(LineCountFilterTest, NonEmptyExcludesZeroLineModels)
{
    const AnalysisModel emptyModel(Path("empty.log"), 0U);
    const AnalysisModel populatedModel(Path("sample.log"), 8U);

    const LineCountFilter filter = LineCountFilter::nonEmpty();

    EXPECT_FALSE(filter.matches(emptyModel));
    EXPECT_TRUE(filter.matches(populatedModel));
}

TEST(LineCountFilterTest, ProgressiveRefinementNarrowsResults)
{
    const AnalysisModel model(Path("sample.log"), 8U);

    const LineCountFilter broadFilter = LineCountFilter::any().withMaximum(10U);
    const LineCountFilter narrowFilter = broadFilter.withMinimum(5U);

    EXPECT_TRUE(broadFilter.matches(model));
    EXPECT_TRUE(narrowFilter.matches(model));
    EXPECT_FALSE(narrowFilter.withMinimum(9U).matches(model));
}
