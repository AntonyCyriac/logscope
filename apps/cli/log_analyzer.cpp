/**
 * @file log_analyzer.cpp
 * @brief LogAnalyzer implementation.
 */

#include "log_analyzer.hpp"

#include "analysis.hpp"
#include "investigation.hpp"
#include "log_macros.hpp"
#include "report_output.hpp"
#include "report_writer.hpp"
#include "source.hpp"

namespace scope::cli
{

bool LogAnalyzer::analyze(const foundation::Path& filePath,
                          const reporting::ReportOptions& reportOptions,
                          const scope::analysis::AnalysisConfig& analysisConfig,
                          const std::optional<foundation::Path>& outputFile,
                          std::ostream& output,
                          std::ostream& errorOutput)
{
    SCOPE_LOG_INFO("cli", "Starting analysis for " + filePath.string());

    scope::source::SourceManager sourceManager;

    auto datasetResult = sourceManager.open(filePath);

    if (!datasetResult)
    {
        const std::string message = datasetResult.error().message();
        SCOPE_LOG_ERROR("cli", message);
        errorOutput << message << '\n';

        return false;
    }

    scope::analysis::AnalysisEngine analysisEngine;

    auto modelResult = analysisEngine.analyze(*datasetResult, analysisConfig);

    if (!modelResult)
    {
        const std::string message = modelResult.error().message();
        SCOPE_LOG_ERROR("cli", message);
        errorOutput << message << '\n';

        return false;
    }

    scope::investigation::InvestigationEngine investigationEngine;

    const scope::investigation::InvestigationView view = investigationEngine.inspect(*modelResult);

    SCOPE_LOG_INFO("cli", "Investigation summary: " + view.summary());

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
