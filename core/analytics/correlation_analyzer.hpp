/**
 * @file correlation_analyzer.hpp
 * @brief Repeated errors and shared correlation identifiers.
 */

#pragma once

#include "analysis_model.hpp"
#include "correlation_result.hpp"

namespace scope::analytics
{

/**
 * @brief Finds repeated errors and shared correlation IDs in indexed lines.
 */
class CorrelationAnalyzer
{
  public:
    [[nodiscard]] CorrelationResult analyze(const analysis::AnalysisModel& model) const;
};

} // namespace scope::analytics
