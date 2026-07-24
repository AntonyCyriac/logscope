/**
 * @file trend_analyzer.cpp
 * @brief TrendAnalyzer implementation.
 */

#include "trend_analyzer.hpp"

#include <cmath>

namespace scope::analytics
{

TrendResult TrendAnalyzer::analyze(const TimelineResult& timeline) const
{
    TrendResult result;

    const std::vector<TimelineBucket>& buckets = timeline.buckets();

    if (buckets.size() < 2U)
    {
        return result;
    }

    const std::size_t midpoint = buckets.size() / 2U;

    std::uint64_t firstErrors = 0U;
    std::uint64_t firstTotal = 0U;
    std::uint64_t secondErrors = 0U;
    std::uint64_t secondTotal = 0U;

    for (std::size_t index = 0U; index < buckets.size(); ++index)
    {
        if (index < midpoint)
        {
            firstErrors += buckets[index].levelCounts.errorLines();
            firstTotal += buckets[index].totalLines;
        }
        else
        {
            secondErrors += buckets[index].levelCounts.errorLines();
            secondTotal += buckets[index].totalLines;
        }
    }

    const double firstRate =
        firstTotal == 0U ? 0.0 : (100.0 * static_cast<double>(firstErrors)) / static_cast<double>(firstTotal);
    const double secondRate =
        secondTotal == 0U ? 0.0 : (100.0 * static_cast<double>(secondErrors)) / static_cast<double>(secondTotal);

    result.setFirstHalfErrorRate(firstRate);
    result.setSecondHalfErrorRate(secondRate);

    if (firstRate > 0.0)
    {
        result.setRateChangePercent(((secondRate - firstRate) / firstRate) * 100.0);
    }
    else if (secondRate > 0.0)
    {
        result.setRateChangePercent(100.0);
    }

    double totalErrors = 0.0;

    for (const TimelineBucket& bucket : buckets)
    {
        totalErrors += static_cast<double>(bucket.levelCounts.errorLines());
    }

    const double averageErrors = totalErrors / static_cast<double>(buckets.size());
    bool hasSpike = false;

    for (const TimelineBucket& bucket : buckets)
    {
        if (averageErrors > 0.0 &&
            static_cast<double>(bucket.levelCounts.errorLines()) > (averageErrors * 2.0))
        {
            hasSpike = true;

            break;
        }
    }

    result.setHasSpike(hasSpike);

    if (hasSpike)
    {
        result.setVerdict("Spike detected: error burst above timeline average");
    }
    else if (secondRate > firstRate + 1.0)
    {
        result.setVerdict("Trending worse: error rate increased in second half");
    }
    else if (firstRate > secondRate + 1.0)
    {
        result.setVerdict("Trending better: error rate decreased in second half");
    }
    else
    {
        result.setVerdict("Stable: no significant error-rate shift");
    }

    return result;
}

} // namespace scope::analytics
