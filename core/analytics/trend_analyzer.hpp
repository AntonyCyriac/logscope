/**
 * @file trend_analyzer.hpp
 * @brief Trend and spike detection over timeline buckets.
 */

#pragma once

#include "timeline_result.hpp"
#include "trend_result.hpp"

namespace scope::analytics
{

/**
 * @brief Derives trend metrics from timeline buckets.
 */
class TrendAnalyzer
{
  public:
    [[nodiscard]] TrendResult analyze(const TimelineResult& timeline) const;
};

} // namespace scope::analytics
