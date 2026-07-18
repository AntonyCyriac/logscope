/**
 * @file extensions_command.hpp
 * @brief Extensions CLI subcommands.
 */

#pragma once

#include <iosfwd>

#include "cli_parser.hpp"
#include "configuration_manager.hpp"

namespace scope::cli
{

void printExtensionsListUsage(std::ostream& output);

void printExtensionsDescribeUsage(std::ostream& output);

[[nodiscard]] int runExtensionsListCommand(const ExtensionsListOptions& options,
                                           configuration::ConfigurationManager& configurationManager,
                                           std::ostream& output,
                                           std::ostream& errorOutput);

[[nodiscard]] int runExtensionsDescribeCommand(const ExtensionsDescribeOptions& options,
                                               configuration::ConfigurationManager& configurationManager,
                                               std::ostream& output,
                                               std::ostream& errorOutput);

} // namespace scope::cli
