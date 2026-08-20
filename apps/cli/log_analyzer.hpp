/**
 * @file log_analyzer.hpp
 * @brief CLI log file analyzer.
 */

#pragma once

#include <iosfwd>
#include <optional>

#include "analysis.hpp"
#include "foundation/path.hpp"
#include "report_options.hpp"
#include "source_open_options.hpp"

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
     * @return 0 on success, 1 on error, 2 when the result is indeterminate.
     */
    [[nodiscard]] int analyze(const foundation::Path& filePath,
                            const reporting::ReportOptions& reportOptions,
                            const scope::analysis::AnalysisConfig& analysisConfig,
                            const std::optional<foundation::Path>& outputFile,
                            std::ostream& output,
                            std::ostream& errorOutput,
                            scope::analysis::AnalysisStats* statsOut = nullptr,
                            const scope::source::OpenOptions& openOptions = {});
};

} // namespace scope::cli
