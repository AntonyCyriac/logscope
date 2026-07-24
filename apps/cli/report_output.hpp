/**
 * @file report_output.hpp
 * @brief Formats analysis results for CLI output.
 */

#pragma once

#include "analysis_model.hpp"
#include "report.hpp"
#include "report_options.hpp"

namespace scope::cli
{

/**
 * @brief Generates a report for the requested output format.
 */
[[nodiscard]] reporting::Report generateAnalysisReport(const analysis::AnalysisModel& model,
                                                       const reporting::ReportOptions& options);

} // namespace scope::cli
