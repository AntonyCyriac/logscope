/**
 * @file session_command.hpp
 * @brief Session CLI subcommands.
 */

#pragma once

#include <iosfwd>

#include "cli_parser.hpp"
#include "configuration_manager.hpp"

namespace scope::cli
{

void printSessionSaveUsage(std::ostream& output);

void printSessionLoadUsage(std::ostream& output);

void printSessionListUsage(std::ostream& output);

[[nodiscard]] int runSessionSaveCommand(const SessionSaveOptions& options,
                                        configuration::ConfigurationManager& configurationManager,
                                        std::ostream& output,
                                        std::ostream& errorOutput);

[[nodiscard]] int runSessionLoadCommand(const SessionLoadOptions& options,
                                        std::ostream& output,
                                        std::ostream& errorOutput);

[[nodiscard]] int runSessionListCommand(const SessionListOptions& options,
                                        std::ostream& output,
                                        std::ostream& errorOutput);

} // namespace scope::cli
