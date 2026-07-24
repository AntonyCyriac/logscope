/**
 * @file timeline_analyzer.hpp
 * @brief Timeline bucket analysis.
 */

#pragma once

#include "analytics_config.hpp"
#include "analysis_model.hpp"
#include "timeline_result.hpp"

namespace scope::analytics
{

/**
 * @brief Builds time-bucket histograms from indexed lines.
 */
class TimelineAnalyzer
{
  public:
    [[nodiscard]] TimelineResult analyze(const analysis::AnalysisModel& model,
                                         const AnalyticsConfig& config) const;
};

} // namespace scope::analytics
