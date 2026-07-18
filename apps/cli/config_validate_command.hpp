/**
 * @file config_validate_command.hpp
 * @brief Config validate subcommand handler.
 */

#pragma once

#include <iostream>

#include "cli_parser.hpp"
#include "configuration_manager.hpp"

namespace scope::cli
{

/**
 * @brief Runs the config validate command.
 */
[[nodiscard]] int runConfigValidateCommand(const ConfigValidateOptions& options,
                                           configuration::ConfigurationManager& configurationManager,
                                           std::ostream& output,
                                           std::ostream& errorOutput);

/**
 * @brief Prints config validate command usage.
 */
void printConfigValidateUsage(std::ostream& output);

} // namespace scope::cli
