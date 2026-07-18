/**
 * @file LogAnalyzer.hpp
 * @brief CLI log file analyzer.
 */

#pragma once

#include <iosfwd>

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
     *
     * @param filePath Path to the log source.
     * @param reportOptions Report format and section selection.
     * @param output Stream that receives formatted output.
     * @return true if analysis succeeded.
     */
    bool analyze(const foundation::Path& filePath,
                 const reporting::ReportOptions& reportOptions,
                 std::ostream& output);
};

} // namespace scope::cli
