/**
 * @file investigate_command.hpp
 * @brief Investigate subcommand for content-aware log investigation.
 */

#pragma once

#include <iosfwd>

#include "cli_parser.hpp"
#include "configuration_manager.hpp"

namespace scope::cli
{

void printInvestigateUsage(std::ostream& output);

int runInvestigateCommand(const InvestigateOptions& options,
                          configuration::ConfigurationManager& configurationManager,
                          std::ostream& output,
                          std::ostream& errorOutput);

} // namespace scope::cli
