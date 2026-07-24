/**
 * @file frequency_analyzer.hpp
 * @brief Frequency analysis over indexed log lines.
 */

#pragma once

#include "analytics_config.hpp"
#include "analysis_model.hpp"
#include "frequency_result.hpp"

namespace scope::analytics
{

/**
 * @brief Computes frequency tables from an analysis model.
 */
class FrequencyAnalyzer
{
  public:
    [[nodiscard]] FrequencyResult analyze(const analysis::AnalysisModel& model,
                                          const AnalyticsConfig& config) const;
};

} // namespace scope::analytics
