/**
 * @file main.cpp
 * @brief LogScope CLI entry point.
 */

#include <iostream>

#include "LogAnalyzer.hpp"

#include "foundation/path.hpp"
#include "runtime.hpp"

namespace
{

void initializeDiagnostics()
{
    scope::runtime::Configuration configuration;

    scope::runtime::Diagnostics::instance().applyConfiguration(configuration);
}

} // namespace

int main(int argc, char* argv[])
{
    initializeDiagnostics();

    if (argc != 2)
    {
        SCOPE_LOG_ERROR("cli", "Usage: logscope <log-file>");

        return 1;
    }

    scope::cli::LogAnalyzer analyzer;

    if (!analyzer.analyze(scope::foundation::Path(argv[1])))
    {
        return 1;
    }

    return 0;
}
