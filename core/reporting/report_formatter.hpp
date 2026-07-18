/**
 * @file report_formatter.hpp
 * @brief Formats analysis models into reports.
 */

#pragma once

#include "analysis_model.hpp"
#include "report.hpp"
#include "report_options.hpp"

namespace scope::reporting
{

/**
 * @brief Formats analysis results into structured reports.
 */
class ReportFormatter
{
  public:
    /**
     * @brief Formats an analysis model using the given options.
     */
    [[nodiscard]] static Report format(const analysis::AnalysisModel& model, const ReportOptions& options);
};

} // namespace scope::reporting
