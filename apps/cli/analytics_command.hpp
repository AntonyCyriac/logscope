/**
 * @file analytics_command.hpp
 * @brief Analytics subcommand.
 */

#pragma once

#include <iosfwd>

#include "analytics_config.hpp"
#include "configuration_manager.hpp"
#include "foundation/path.hpp"
#include "log_format.hpp"
#include "output_format.hpp"

namespace scope::cli
{

/**
 * @brief Options for the analytics command.
 */
struct AnalyticsOptions
{
    foundation::Path logFile;
    foundation::Path configFile;
    OutputFormat format = OutputFormat::Text;
    analysis::LogFormat logFormat = analysis::LogFormat::Auto;
    std::string profile;
    analytics::AnalyticsConfig analyticsConfig;
    bool showHelp = false;
};

void printAnalyticsUsage(std::ostream& output);

int runAnalyticsCommand(const AnalyticsOptions& options,
                        configuration::ConfigurationManager& configurationManager,
                        std::ostream& output,
                        std::ostream& errorOutput);

} // namespace scope::cli
