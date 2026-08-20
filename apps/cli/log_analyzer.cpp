/**
 * @file log_analyzer.cpp
 * @brief LogAnalyzer implementation.
 */

#include "log_analyzer.hpp"

#include "application_service.hpp"
#include "foundation/error.hpp"
#include "log_macros.hpp"
#include "report_output.hpp"
#include "report_writer.hpp"

namespace scope::cli
{

namespace
{

[[nodiscard]] int exitCodeForError(const foundation::Error& error) noexcept
{
    if (error.code() == foundation::ErrorCode::Indeterminate)
    {
        return 2;
    }

    return 1;
}

} // namespace

int LogAnalyzer::analyze(const foundation::Path& filePath,
                       const reporting::ReportOptions& reportOptions,
                       const scope::analysis::AnalysisConfig& analysisConfig,
                       const std::optional<foundation::Path>& outputFile,
                       std::ostream& output,
                       std::ostream& errorOutput,
                       scope::analysis::AnalysisStats* statsOut,
                       const scope::source::OpenOptions& openOptions)
{
    SCOPE_LOG_INFO("cli", "Starting analysis for " + filePath.string());

    scope::application::ApplicationService service;

    const auto openResult = service.openSource(filePath, openOptions);

    if (!openResult)
    {
        errorOutput << openResult.error().message() << '\n';

        return exitCodeForError(openResult.error());
    }

    const auto modelResult = service.analyze(analysisConfig, statsOut);

    if (!modelResult)
    {
        errorOutput << modelResult.error().message() << '\n';

        return exitCodeForError(modelResult.error());
    }

    SCOPE_LOG_INFO("cli", "Analysis complete for " + filePath.string());

    const reporting::Report report = generateAnalysisReport(*modelResult, reportOptions);
    const auto writeResult = writeReport(report, outputFile, output, errorOutput);

    if (!writeResult)
    {
        errorOutput << writeResult.error().message() << '\n';

        return 1;
    }

    return 0;
}

} // namespace scope::cli
