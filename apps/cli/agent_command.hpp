/**
 * @file agent_command.hpp
 * @brief AI agent subcommands (M13).
 */

#pragma once

#include <iosfwd>

#include "cli_parser.hpp"
#include "configuration_manager.hpp"

namespace scope::cli
{

void printAgentUsage(std::ostream& output);

void printAgentInvestigateUsage(std::ostream& output);

int runAgentInvestigateCommand(const AgentInvestigateOptions& options,
                               configuration::ConfigurationManager& configurationManager,
                               std::ostream& output,
                               std::ostream& errorOutput);

} // namespace scope::cli
