/**
 * @file LogAnalyzer.hpp
 * @brief CLI log file analyzer.
 */

#pragma once

#include <iosfwd>

#include "foundation/path.hpp"
#include "log_format.hpp"
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
     *
     * @param filePath Path to the log source.
     * @param reportOptions Report format and section selection.
     * @param logFormat Input format hint (auto detects by default).
     * @param output Stream that receives formatted output.
     * @param errorOutput Stream that receives actionable error messages.
     * @return true if analysis succeeded.
     */
    bool analyze(const foundation::Path& filePath,
                 const reporting::ReportOptions& reportOptions,
                 analysis::LogFormat logFormat,
                 std::ostream& output,
                 std::ostream& errorOutput);
};

} // namespace scope::cli
