/**
 * @file report_output.hpp
 * @brief Formats analysis results for CLI output.
 */

#pragma once

#include <string>

#include "analysis_model.hpp"
#include "report_options.hpp"

namespace scope::cli
{

/**
 * @brief Formats an analysis model for the requested output format.
 */
[[nodiscard]] std::string formatAnalysisOutput(const analysis::AnalysisModel& model,
                                               const reporting::ReportOptions& options);

} // namespace scope::cli
