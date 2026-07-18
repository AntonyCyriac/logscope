/**
 * @file LogAnalyzer.cpp
 * @brief LogAnalyzer implementation.
 */

#include "LogAnalyzer.hpp"

#include <fstream>
#include <iostream>

#include "foundation/filesystem.hpp"
#include "log_macros.hpp"

namespace scope::cli
{

bool LogAnalyzer::analyze(const foundation::Path& filePath)
{
    SCOPE_LOG_INFO("cli", "Starting analysis for " + filePath.string());

    auto existsResult = foundation::FileSystem::exists(filePath);

    if (!existsResult || !*existsResult)
    {
        SCOPE_LOG_ERROR("cli", "Unable to open log file: " + filePath.string());

        return false;
    }

    std::ifstream logFile(filePath.string());

    if (!logFile.is_open())
    {
        SCOPE_LOG_ERROR("cli", "Unable to open log file: " + filePath.string());

        return false;
    }

    SCOPE_LOG_DEBUG("cli", "Opened log file: " + filePath.string());

    std::string line;
    unsigned long totalLines = 0;

    while (std::getline(logFile, line))
    {
        ++totalLines;
    }

    SCOPE_LOG_INFO("cli", "Counted " + std::to_string(totalLines) + " log lines");

    std::cout << "========== LOGSCOPE REPORT ==========" << std::endl;
    std::cout << "Total log lines : " << totalLines << std::endl;
    std::cout << "=====================================" << std::endl;

    return true;
}

} // namespace scope::cli
