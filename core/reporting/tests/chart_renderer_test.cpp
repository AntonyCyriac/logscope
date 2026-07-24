/**
 * @file chart_renderer_test.cpp
 * @brief Unit tests for chart renderers.
 */

#include <gtest/gtest.h>

#include "ascii_chart_renderer.hpp"
#include "chart_model.hpp"
#include "log_level_counts.hpp"
#include "svg_chart_renderer.hpp"

using scope::analysis::LogLevelCounts;
using scope::reporting::buildLevelBarChart;
using scope::reporting::renderAsciiLevelChart;
using scope::reporting::renderSvgLevelChart;

TEST(ChartRendererTest, AsciiChartReflectsLevelProportions)
{
    LogLevelCounts levels;
    levels.recordError();
    levels.recordError();
    levels.recordWarn();
    levels.recordInfo();

    const auto chart = buildLevelBarChart(levels);
    const std::string ascii = renderAsciiLevelChart(chart);

    EXPECT_NE(std::string::npos, ascii.find("Error"));
    EXPECT_NE(std::string::npos, ascii.find("2"));
    EXPECT_NE(std::string::npos, ascii.find("Warning"));
    EXPECT_NE(std::string::npos, ascii.find("Info"));
}

TEST(ChartRendererTest, SvgChartContainsExpectedDimensions)
{
    LogLevelCounts levels;
    levels.recordError();
    levels.recordInfo();

    const auto chart = buildLevelBarChart(levels);
    const std::string svg = renderSvgLevelChart(chart);

    EXPECT_NE(std::string::npos, svg.find("<svg"));
    EXPECT_NE(std::string::npos, svg.find("width=\"480\""));
    EXPECT_NE(std::string::npos, svg.find("Error"));
    EXPECT_NE(std::string::npos, svg.find("#dc3545"));
}
