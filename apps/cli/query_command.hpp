/**
 * @file query_command.hpp
 * @brief Query subcommand for field-aware filter DSL.
 */

#pragma once

#include <iosfwd>

#include "cli_parser.hpp"
#include "configuration_manager.hpp"

namespace scope::cli
{

void printQueryUsage(std::ostream& output);

int runQueryCommand(const InvestigateOptions& options,
                    configuration::ConfigurationManager& configurationManager,
                    std::ostream& output,
                    std::ostream& errorOutput);

} // namespace scope::cli
