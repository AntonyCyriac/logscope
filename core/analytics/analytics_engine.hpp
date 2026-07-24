/**
 * @file analytics_engine.hpp
 * @brief Orchestrates analytics analyzers over an analysis model.
 */

#pragma once

#include "analytics_config.hpp"
#include "analytics_result.hpp"
#include "analysis_model.hpp"

namespace scope::analytics
{

/**
 * @brief Runs frequency, clustering, timeline, trend, and correlation analytics.
 */
class AnalyticsEngine
{
  public:
    [[nodiscard]] AnalyticsResult analyze(const analysis::AnalysisModel& model,
                                          const AnalyticsConfig& config = {}) const;
};

} // namespace scope::analytics
