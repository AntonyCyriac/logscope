/**
 * @file LogAnalyzer.cpp
 * @brief LogAnalyzer implementation.
 */

#include "LogAnalyzer.hpp"

#include <fstream>
#include <iostream>

#include "foundation/filesystem.hpp"

namespace scope::cli
{

bool LogAnalyzer::analyze(const foundation::Path& filePath)
{
    auto existsResult = foundation::FileSystem::exists(filePath);

    if (!existsResult || !*existsResult)
    {
        std::cerr << "Error: Unable to open log file: " << filePath.string() << std::endl;

        return false;
    }

    std::ifstream logFile(filePath.string());

    if (!logFile.is_open())
    {
        std::cerr << "Error: Unable to open log file: " << filePath.string() << std::endl;

        return false;
    }

    std::string line;
    unsigned long totalLines = 0;

    while (std::getline(logFile, line))
    {
        ++totalLines;
    }

    std::cout << "========== LOGSCOPE REPORT ==========" << std::endl;
    std::cout << "Total log lines : " << totalLines << std::endl;
    std::cout << "=====================================" << std::endl;

    return true;
}

} // namespace scope::cli
