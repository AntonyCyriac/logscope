/**
 * @file chart_model.hpp
 * @brief Chart datasets derived from analysis results.
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "log_level_counts.hpp"

namespace scope::analytics
{

class TimelineResult;

} // namespace scope::analytics

namespace scope::reporting
{

/**
 * @brief One bar in a horizontal bar chart.
 */
struct ChartBar
{
    std::string label;
    std::uint64_t value{0U};
    std::string color;
};

/**
 * @brief Level breakdown bar chart dataset.
 */
struct LevelBarChart
{
    std::vector<ChartBar> bars;

    [[nodiscard]] std::uint64_t maxValue() const noexcept;
};

/**
 * @brief One point in a time-series chart bucket.
 */
struct TimeSeriesPoint
{
    std::string label;
    std::uint64_t errorCount{0U};
    std::uint64_t totalCount{0U};
};

/**
 * @brief Error counts over timeline buckets.
 */
struct TimeSeriesChart
{
    std::vector<TimeSeriesPoint> points;

    [[nodiscard]] std::uint64_t maxValue() const noexcept;
};

/**
 * @brief Builds a level bar chart from log level counts.
 */
[[nodiscard]] LevelBarChart buildLevelBarChart(const analysis::LogLevelCounts& levels);

/**
 * @brief Builds a time-series chart from analytics timeline output.
 */
[[nodiscard]] TimeSeriesChart buildTimelineChart(const analytics::TimelineResult& timeline);

} // namespace scope::reporting
