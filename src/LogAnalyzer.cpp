#include "LogAnalyzer.h"

#include <fstream>
#include <iostream>
#include <string>

bool LogAnalyzer::analyze(const std::string& filePath)
{
    std::ifstream logFile(filePath.c_str());

    if (!logFile.is_open())
    {
        std::cerr << "Error: Unable to open log file: "
                  << filePath << std::endl;

        return false;
    }

    std::string line;
    unsigned long totalLines = 0;

    while (std::getline(logFile, line))
    {
        ++totalLines;
    }

    std::cout << "========== LOGSCOPE REPORT =========="
              << std::endl;

    std::cout << "Total log lines : "
              << totalLines << std::endl;

    std::cout << "====================================="
              << std::endl;

    return true;
}
