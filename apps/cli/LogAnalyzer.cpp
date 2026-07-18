/**
 * @file LogAnalyzer.cpp
 * @brief LogAnalyzer implementation.
 */

#include "LogAnalyzer.hpp"

#include <iostream>

#include "analysis.hpp"
#include "log_macros.hpp"
#include "source.hpp"

namespace scope::cli
{

bool LogAnalyzer::analyze(const foundation::Path& filePath)
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

    std::cout << "========== LOGSCOPE REPORT ==========" << std::endl;
    std::cout << "Source          : " << modelResult->sourcePath().string() << std::endl;
    std::cout << "Total log lines : " << modelResult->totalLines() << std::endl;
    std::cout << "=====================================" << std::endl;

    return true;
}

} // namespace scope::cli
