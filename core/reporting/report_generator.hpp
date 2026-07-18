/**
 * @file report_generator.hpp
 * @brief Generates reports from analysis models.
 */

#pragma once

#include "analysis_model.hpp"
#include "report.hpp"
#include "report_options.hpp"

namespace scope::reporting
{

/**
 * @brief Generates reusable reports from analysis results.
 */
class ReportGenerator
{
  public:
    /**
     * @brief Creates a report from an analysis model.
     *
     * @param model Analysis model to present.
     * @param options Report format and section selection.
     * @return Formatted report.
     */
    [[nodiscard]] Report generate(const analysis::AnalysisModel& model,
                                  const ReportOptions& options = ReportOptions::defaults()) const;
};

} // namespace scope::reporting
