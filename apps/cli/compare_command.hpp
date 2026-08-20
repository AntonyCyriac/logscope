/**
 * @file compare_command.hpp
 * @brief Compare subcommand.
 */

#pragma once

#include <iosfwd>

#include "cli_parser.hpp"
#include "configuration_manager.hpp"

namespace scope::cli
{

void printCompareUsage(std::ostream& output);

[[nodiscard]] int runCompareCommand(const CompareOptions& options,
                                    configuration::ConfigurationManager& configurationManager,
                                    std::ostream& output,
                                    std::ostream& errorOutput);

} // namespace scope::cli
