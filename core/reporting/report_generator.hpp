/**
 * @file report_generator.hpp
 * @brief Generates reports from analysis models.
 */

#pragma once

#include "analysis_model.hpp"
#include "report.hpp"

namespace scope::reporting
{

/**
 * @brief Generates reusable reports from analysis results.
 */
class ReportGenerator
{
  public:
    /**
     * @brief Creates a text report from an analysis model.
     *
     * @param model Analysis model to present.
     * @return Formatted report.
     */
    [[nodiscard]] Report generate(const analysis::AnalysisModel& model) const;
};

} // namespace scope::reporting
