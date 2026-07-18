/**
 * @file LogAnalyzer.cpp
 * @brief LogAnalyzer implementation.
 */

#include "LogAnalyzer.hpp"

#include "analysis.hpp"
#include "investigation.hpp"
#include "log_macros.hpp"
#include "report_output.hpp"
#include "source.hpp"

namespace scope::cli
{

bool LogAnalyzer::analyze(const foundation::Path& filePath, const OutputFormat format, std::ostream& output)
{
    SCOPE_LOG_INFO("cli", "Starting analysis for " + filePath.string());

    scope::source::SourceManager sourceManager;

    auto datasetResult = sourceManager.open(filePath);

    if (!datasetResult)
    {
        SCOPE_LOG_ERROR("cli", datasetResult.error().message());

        return false;
    }

    scope::analysis::AnalysisEngine analysisEngine;

    auto modelResult = analysisEngine.analyze(*datasetResult);

    if (!modelResult)
    {
        SCOPE_LOG_ERROR("cli", modelResult.error().message());

        return false;
    }

    scope::investigation::InvestigationEngine investigationEngine;

    const scope::investigation::InvestigationView view = investigationEngine.inspect(*modelResult);

    SCOPE_LOG_INFO("cli", "Investigation summary: " + view.summary());

    output << formatAnalysisOutput(*modelResult, format) << std::endl;

    return true;
}

} // namespace scope::cli
