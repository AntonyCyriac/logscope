/**
 * @file ascii_chart_renderer.hpp
 * @brief ASCII horizontal bar charts for text and markdown reports.
 */

#pragma once

#include <string>

#include "chart_model.hpp"

namespace scope::reporting
{

/**
 * @brief Renders a level bar chart as plain text.
 */
[[nodiscard]] std::string renderAsciiLevelChart(const LevelBarChart& chart, std::size_t maxBarWidth = 40U);

/**
 * @brief Renders a level bar chart as a markdown fenced code block.
 */
[[nodiscard]] std::string renderMarkdownLevelChart(const LevelBarChart& chart, std::size_t maxBarWidth = 40U);

} // namespace scope::reporting
