/**
 * @file svg_chart_renderer.cpp
 * @brief SVG bar charts for HTML and PDF reports.
 */

#include "svg_chart_renderer.hpp"

#include <algorithm>
#include <sstream>

namespace scope::reporting
{

std::string renderSvgLevelChart(const LevelBarChart& chart)
{
    std::ostringstream output;

    constexpr int width = 480;
    constexpr int barHeight = 24;
    constexpr int gap = 8;
    constexpr int labelWidth = 72;
    constexpr int chartLeft = labelWidth + 8;
    constexpr int chartWidth = width - chartLeft - 48;
    const int height = static_cast<int>(chart.bars.size()) * (barHeight + gap) + 16;

    output << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << width << "\" height=\"" << height
           << "\" viewBox=\"0 0 " << width << ' ' << height << "\" role=\"img\" aria-label=\"Log level chart\">\n";

    const std::uint64_t maximum = std::max<std::uint64_t>(chart.maxValue(), 1U);
    int y = 8;

    for (const ChartBar& bar : chart.bars)
    {
        const int barPixelWidth =
            static_cast<int>((bar.value * static_cast<std::uint64_t>(chartWidth)) / maximum);

        output << "  <text x=\"0\" y=\"" << (y + 16) << "\" font-family=\"sans-serif\" font-size=\"12\">"
               << bar.label << "</text>\n";
        output << "  <rect x=\"" << chartLeft << "\" y=\"" << y << "\" width=\"" << barPixelWidth
               << "\" height=\"" << barHeight << "\" fill=\"" << bar.color << "\" rx=\"2\"/>\n";
        output << "  <text x=\"" << (chartLeft + barPixelWidth + 6) << "\" y=\"" << (y + 16)
               << "\" font-family=\"sans-serif\" font-size=\"12\">" << bar.value << "</text>\n";

        y += barHeight + gap;
    }

    output << "</svg>\n";

    return output.str();
}

} // namespace scope::reporting
