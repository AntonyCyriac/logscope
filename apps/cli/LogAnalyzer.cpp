/**
 * @file LogAnalyzer.cpp
 * @brief LogAnalyzer implementation.
 */

#include "LogAnalyzer.hpp"

#include <iostream>

#include "log_macros.hpp"
#include "source.hpp"

using scope::source::LogSource;

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

    std::string line;
    unsigned long totalLines = 0;
    LogSource& logSource = datasetResult->source();

    while (true)
    {
        const auto readResult = logSource.readLine(line);

        if (!readResult)
        {
            SCOPE_LOG_ERROR("cli", readResult.error().message());

            return false;
        }

        if (!*readResult)
        {
            break;
        }

        ++totalLines;
    }

    SCOPE_LOG_INFO("cli", "Counted " + std::to_string(totalLines) + " log lines");

    std::cout << "========== LOGSCOPE REPORT ==========" << std::endl;
    std::cout << "Total log lines : " << totalLines << std::endl;
    std::cout << "=====================================" << std::endl;

    return true;
}

} // namespace scope::cli
