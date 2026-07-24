/**
 * @file trend_analyzer_test.cpp
 */

#include <gtest/gtest.h>

#include "timeline_result.hpp"
#include "trend_analyzer.hpp"

using scope::analytics::TimelineBucket;
using scope::analytics::TimelineResult;
using scope::analytics::TrendAnalyzer;
using scope::analysis::LogLevelCounts;
using scope::foundation::Timestamp;

TEST(TrendAnalyzerTest, DetectsWorseningTrend)
{
    TimelineResult timeline;
    timeline.setBucketSeconds(60);

    std::vector<TimelineBucket> buckets(4U);

    for (std::size_t index = 0U; index < buckets.size(); ++index)
    {
        buckets[index].start = Timestamp::fromUnixSeconds(static_cast<std::int64_t>(index * 60));
        buckets[index].end = Timestamp::fromUnixSeconds(static_cast<std::int64_t>((index + 1) * 60));
        buckets[index].totalLines = 10U;

        LogLevelCounts counts = LogLevelCounts::fromCounts(0U, 0U, index < 2U ? 1U : 3U, 0U, 0U);

        buckets[index].levelCounts = counts;
    }

    timeline.setBuckets(std::move(buckets));

    const TrendAnalyzer analyzer;
    const auto trend = analyzer.analyze(timeline);

    EXPECT_GT(trend.secondHalfErrorRate(), trend.firstHalfErrorRate());
    EXPECT_NE(std::string::npos, trend.verdict().find("worse"));
}
