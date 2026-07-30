/**
 * @file log_analyzer.cpp
 * @brief LogAnalyzer implementation.
 */

#include "log_analyzer.hpp"

#include "application_service.hpp"
#include "log_macros.hpp"
#include "report_output.hpp"
#include "report_writer.hpp"

namespace scope::cli
{

bool LogAnalyzer::analyze(const foundation::Path& filePath,
                          const reporting::ReportOptions& reportOptions,
                          const scope::analysis::AnalysisConfig& analysisConfig,
                          const std::optional<foundation::Path>& outputFile,
                          std::ostream& output,
                          std::ostream& errorOutput,
                          scope::analysis::AnalysisStats* statsOut)
{
    SCOPE_LOG_INFO("cli", "Starting analysis for " + filePath.string());

    scope::application::ApplicationService service;

    const auto openResult = service.openSource(filePath);

    if (!openResult)
    {
        errorOutput << openResult.error().message() << '\n';

        return false;
    }

    const auto modelResult = service.analyze(analysisConfig, statsOut);

    if (!modelResult)
    {
        errorOutput << modelResult.error().message() << '\n';

        return false;
    }

    SCOPE_LOG_INFO("cli", "Analysis complete for " + filePath.string());

    const reporting::Report report = generateAnalysisReport(*modelResult, reportOptions);
    const auto writeResult = writeReport(report, outputFile, output, errorOutput);

    if (!writeResult)
    {
        errorOutput << writeResult.error().message() << '\n';

        return false;
    }

    return true;
}

} // namespace scope::cli
