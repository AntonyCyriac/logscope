/**
 * @file chart_model.hpp
 * @brief Chart datasets derived from analysis results.
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "log_level_counts.hpp"

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
 * @brief Builds a level bar chart from log level counts.
 */
[[nodiscard]] LevelBarChart buildLevelBarChart(const analysis::LogLevelCounts& levels);

} // namespace scope::reporting
