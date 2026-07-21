/**
 * @file log_analyzer.hpp
 * @brief CLI log file analyzer.
 */

#pragma once

#include <iosfwd>

#include "analysis.hpp"
#include "foundation/path.hpp"
#include "report_options.hpp"

namespace scope::cli
{

/**
 * @brief Analyzes log files and produces a summary report.
 */
class LogAnalyzer
{
  public:
    /**
     * @brief Analyzes the log source at the given path.
     */
    bool analyze(const foundation::Path& filePath,
                 const reporting::ReportOptions& reportOptions,
                 const scope::analysis::AnalysisConfig& analysisConfig,
                 std::ostream& output,
                 std::ostream& errorOutput);
};

} // namespace scope::cli
