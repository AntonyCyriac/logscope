/**
 * @file cli_application.hpp
 * @brief CLI command dispatcher.
 */

#pragma once

#include <iostream>

#include "cli_parser.hpp"
#include "configuration_manager.hpp"

namespace scope::cli
{

/**
 * @brief Dispatches parsed CLI commands.
 */
class CliApplication
{
  public:
    /**
     * @brief Runs the CLI using parsed arguments.
     */
    [[nodiscard]] int run(const ParsedCli& parsed,
                          configuration::ConfigurationManager& configurationManager,
                          std::ostream& output = std::cout,
                          std::ostream& errorOutput = std::cerr) const;

    /**
     * @brief Prints top-level CLI usage.
     */
    static void printUsage(std::ostream& output);

    /**
     * @brief Prints an unknown-command error with an optional suggestion.
     */
    static void printUnknownCommandError(std::ostream& errorOutput, const std::string& command);
};

} // namespace scope::cli
