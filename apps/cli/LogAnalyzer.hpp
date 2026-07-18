/**
 * @file LogAnalyzer.hpp
 * @brief CLI log file analyzer.
 */

#pragma once

#include "foundation/path.hpp"

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
     * @return true if analysis succeeded.
     */
    bool analyze(const foundation::Path& filePath);
};

} // namespace scope::cli
