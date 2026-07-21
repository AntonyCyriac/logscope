/**
 * @file investigation_output.hpp
 * @brief Formats content-aware investigation results for the CLI.
 */

#pragma once

#include <ostream>
#include <string>

#include "investigation_result.hpp"
#include "output_format.hpp"

namespace scope::cli
{

/**
 * @brief Formats an investigation result for CLI output.
 */
[[nodiscard]] std::string formatInvestigationOutput(const investigation::InvestigationResult& result,
                                                    OutputFormat format);

} // namespace scope::cli
