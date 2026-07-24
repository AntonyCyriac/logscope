/**
 * @file chart_model.cpp
 * @brief Chart datasets derived from analysis results.
 */

#include "chart_model.hpp"

#include <algorithm>

#include "timeline_result.hpp"

namespace scope::reporting
{

std::uint64_t LevelBarChart::maxValue() const noexcept
{
    std::uint64_t maximum = 0U;

    for (const ChartBar& bar : bars)
    {
        maximum = std::max(maximum, bar.value);
    }

    return maximum;
}

LevelBarChart buildLevelBarChart(const analysis::LogLevelCounts& levels)
{
    LevelBarChart chart;

    chart.bars.push_back(ChartBar{"Error", levels.errorLines(), "#dc3545"});
    chart.bars.push_back(ChartBar{"Warning", levels.warnLines(), "#ffc107"});
    chart.bars.push_back(ChartBar{"Info", levels.infoLines(), "#0d6efd"});
    chart.bars.push_back(ChartBar{"Other", levels.otherLines(), "#6c757d"});
    chart.bars.push_back(ChartBar{"Blank", levels.blankLines(), "#adb5bd"});

    return chart;
}

std::uint64_t TimeSeriesChart::maxValue() const noexcept
{
    std::uint64_t maximum = 0U;

    for (const TimeSeriesPoint& point : points)
    {
        maximum = std::max(maximum, point.errorCount);
    }

    return maximum;
}

TimeSeriesChart buildTimelineChart(const analytics::TimelineResult& timeline)
{
    TimeSeriesChart chart;

    for (const analytics::TimelineBucket& bucket : timeline.buckets())
    {
        if (bucket.totalLines == 0U)
        {
            continue;
        }

        TimeSeriesPoint point;
        point.label = bucket.label;
        point.errorCount = bucket.levelCounts.errorLines();
        point.totalCount = bucket.totalLines;
        chart.points.push_back(std::move(point));
    }

    return chart;
}

} // namespace scope::reporting
