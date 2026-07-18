/**
 * @file main.cpp
 * @brief LogScope CLI entry point.
 */

#include <iostream>

#include "LogAnalyzer.hpp"

#include "foundation/path.hpp"

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: logscope <log-file>" << std::endl;

        return 1;
    }

    scope::cli::LogAnalyzer analyzer;

    if (!analyzer.analyze(scope::foundation::Path(argv[1])))
    {
        return 1;
    }

    return 0;
}
