/**
 * @file svg_chart_renderer.hpp
 * @brief SVG bar charts for HTML and PDF reports.
 */

#pragma once

#include <string>

#include "chart_model.hpp"

namespace scope::reporting
{

/**
 * @brief Renders a level bar chart as an inline SVG fragment.
 */
[[nodiscard]] std::string renderSvgLevelChart(const LevelBarChart& chart);

/**
 * @brief Renders a time-series chart as an inline SVG fragment.
 */
[[nodiscard]] std::string renderSvgTimeSeriesChart(const TimeSeriesChart& chart);

} // namespace scope::reporting
