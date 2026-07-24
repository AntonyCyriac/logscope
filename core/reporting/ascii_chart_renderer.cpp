/**
 * @file ascii_chart_renderer.cpp
 * @brief ASCII horizontal bar charts for text and markdown reports.
 */

#include "ascii_chart_renderer.hpp"

#include <algorithm>
#include <sstream>

namespace scope::reporting
{

namespace
{

std::string renderAsciiChartBody(const LevelBarChart& chart, const std::size_t maxBarWidth)
{
    std::ostringstream output;

    const std::uint64_t maximum = std::max<std::uint64_t>(chart.maxValue(), 1U);
    constexpr std::size_t labelWidth = 7U;

    for (const ChartBar& bar : chart.bars)
    {
        const std::size_t barLength =
            static_cast<std::size_t>((bar.value * maxBarWidth) / maximum);

        output << bar.label;

        if (bar.label.size() < labelWidth)
        {
            output << std::string(labelWidth - bar.label.size(), ' ');
        }

        output << " |" << std::string(barLength, '#') << ' ' << bar.value << '\n';
    }

    return output.str();
}

} // namespace

std::string renderAsciiLevelChart(const LevelBarChart& chart, const std::size_t maxBarWidth)
{
    return renderAsciiChartBody(chart, maxBarWidth);
}

std::string renderMarkdownLevelChart(const LevelBarChart& chart, const std::size_t maxBarWidth)
{
    std::ostringstream output;

    output << "```\n" << renderAsciiChartBody(chart, maxBarWidth) << "```\n";

    return output.str();
}

std::string renderAsciiTimeSeriesChart(const TimeSeriesChart& chart, const std::size_t maxBarWidth)
{
    LevelBarChart converted;

    for (const TimeSeriesPoint& point : chart.points)
    {
        converted.bars.push_back(ChartBar{point.label, point.errorCount, "#dc3545"});
    }

    return renderAsciiChartBody(converted, maxBarWidth);
}

std::string renderMarkdownTimeSeriesChart(const TimeSeriesChart& chart, const std::size_t maxBarWidth)
{
    std::ostringstream output;

    output << "```\n" << renderAsciiTimeSeriesChart(chart, maxBarWidth) << "```\n";

    return output.str();
}

} // namespace scope::reporting
