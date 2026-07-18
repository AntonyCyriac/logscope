/**
 * @file report_config.hpp
 * @brief Builds report options from CLI and configuration inputs.
 */

#pragma once

#include "cli_parser.hpp"
#include "configuration_manager.hpp"
#include "report_options.hpp"

namespace scope::cli
{

/**
 * @brief Builds report options from analyze options and loaded configuration.
 */
[[nodiscard]] reporting::ReportOptions buildReportOptions(
    const AnalyzeOptions& options, const configuration::ConfigurationManager& configurationManager);

} // namespace scope::cli
