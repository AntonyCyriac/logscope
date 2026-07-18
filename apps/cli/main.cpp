/**
 * @file main.cpp
 * @brief LogScope CLI entry point.
 */

#include <iostream>

#include "cli_application.hpp"
#include "cli_parser.hpp"
#include "configuration_manager.hpp"

int main(int argc, char* argv[])
{
    const auto parsed = scope::cli::parseCliArguments(argc, argv);

    if (!parsed)
    {
        scope::cli::CliApplication::printUsage(std::cerr);

        return 1;
    }

    scope::configuration::ConfigurationManager configurationManager;

    const scope::cli::CliApplication application;

    return application.run(*parsed, configurationManager);
}
