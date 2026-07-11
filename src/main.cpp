#include "LogAnalyzer.h"

#include <iostream>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: logscope <log-file>"
                  << std::endl;

        return 1;
    }

    LogAnalyzer analyzer;

    if (!analyzer.analyze(argv[1]))
    {
        return 1;
    }

    return 0;
}
