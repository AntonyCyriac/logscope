/**
 * @file cli_analysis_config.hpp
 * @brief CLI helpers for resolving analysis configuration.
 */

#pragma once

#include "analysis.hpp"
#include "analytics_command.hpp"
#include "cli_parser.hpp"
#include "configuration_manager.hpp"

namespace scope::cli
{

/**
 * @brief Builds analysis settings from CLI options and loaded configuration.
 */
[[nodiscard]] scope::analysis::AnalysisConfig buildAnalysisConfig(const AnalyzeOptions& options,
                                                                const configuration::ConfigurationManager&
                                                                    configurationManager);

[[nodiscard]] scope::analysis::AnalysisConfig buildAnalysisConfig(const InvestigateOptions& options,
                                                                const configuration::ConfigurationManager&
                                                                    configurationManager);

[[nodiscard]] scope::analysis::AnalysisConfig buildAnalysisConfig(const SessionSaveOptions& options,
                                                                const configuration::ConfigurationManager&
                                                                    configurationManager);

[[nodiscard]] scope::analysis::AnalysisConfig buildAnalysisConfig(const AnalyticsOptions& options,
                                                                const configuration::ConfigurationManager&
                                                                    configurationManager);

} // namespace scope::cli
