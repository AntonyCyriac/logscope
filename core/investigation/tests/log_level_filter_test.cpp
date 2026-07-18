/**
 * @file log_level_filter_test.cpp
 * @brief Unit tests for LogLevelFilter.
 */

#include <gtest/gtest.h>

#include "analysis.hpp"
#include "investigation.hpp"

using scope::analysis::AnalysisModel;
using scope::analysis::LogLevelCounts;
using scope::foundation::Path;
using scope::investigation::LogLevelFilter;

namespace
{

AnalysisModel createModelWithLevels(const std::uint64_t errors, const std::uint64_t warnings)
{
    LogLevelCounts levelCounts;

    for (std::uint64_t index = 0; index < errors; ++index)
    {
        levelCounts.recordError();
    }

    for (std::uint64_t index = 0; index < warnings; ++index)
    {
        levelCounts.recordWarn();
    }

    return AnalysisModel(Path("sample.log"), errors + warnings, levelCounts);
}

} // namespace

TEST(LogLevelFilterTest, AnyMatchesAllModels)
{
    const AnalysisModel emptyModel(Path("empty.log"), 0U);
    const AnalysisModel populatedModel = createModelWithLevels(2U, 1U);

    const LogLevelFilter filter = LogLevelFilter::any();

    EXPECT_TRUE(filter.matches(emptyModel));
    EXPECT_TRUE(filter.matches(populatedModel));
}

TEST(LogLevelFilterTest, MinimumErrorsExcludesLowErrorModels)
{
    const AnalysisModel model = createModelWithLevels(2U, 0U);

    EXPECT_TRUE(LogLevelFilter::any().withMinimumErrors(2U).matches(model));
    EXPECT_FALSE(LogLevelFilter::any().withMinimumErrors(3U).matches(model));
}

TEST(LogLevelFilterTest, ProgressiveRefinementCombinesThresholds)
{
    const AnalysisModel model = createModelWithLevels(2U, 1U);

    const LogLevelFilter filter = LogLevelFilter::any().withMinimumErrors(1U).withMinimumWarnings(1U);

    EXPECT_TRUE(filter.matches(model));
    EXPECT_FALSE(filter.withMinimumErrors(3U).matches(model));
}
