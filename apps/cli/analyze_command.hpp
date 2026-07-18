/**
 * @file analyze_command.hpp
 * @brief Analyze subcommand handler.
 */

#pragma once

#include <iostream>

#include "cli_parser.hpp"
#include "configuration_manager.hpp"

namespace scope::cli
{

/**
 * @brief Runs the analyze command.
 */
[[nodiscard]] int runAnalyzeCommand(const AnalyzeOptions& options,
                                    configuration::ConfigurationManager& configurationManager,
                                    std::ostream& output,
                                    std::ostream& errorOutput);

/**
 * @brief Prints analyze command usage.
 */
void printAnalyzeUsage(std::ostream& output);

} // namespace scope::cli
