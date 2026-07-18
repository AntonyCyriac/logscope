/**
 * @file LogAnalyzer.hpp
 * @brief CLI log file analyzer.
 */

#pragma once

#include <iosfwd>

#include "foundation/path.hpp"
#include "output_format.hpp"

namespace scope::cli
{

/**
 * @brief Analyzes log files and produces a summary report.
 */
class LogAnalyzer
{
  public:
    /**
     * @brief Analyzes the log file at the given path.
     *
     * @param filePath Path to the log file.
     * @param format Output format for the analysis result.
     * @param output Stream that receives formatted output.
     * @return true if analysis succeeded.
     */
    bool analyze(const foundation::Path& filePath,
                 OutputFormat format,
                 std::ostream& output);
};

} // namespace scope::cli
