/**
 * @file search_command.hpp
 * @brief Search subcommand for indexed log queries.
 */

#pragma once

#include <iosfwd>

#include "cli_parser.hpp"
#include "configuration_manager.hpp"

namespace scope::cli
{

void printSearchUsage(std::ostream& output);

int runSearchCommand(const InvestigateOptions& options,
                     configuration::ConfigurationManager& configurationManager,
                     std::ostream& output,
                     std::ostream& errorOutput);

} // namespace scope::cli
