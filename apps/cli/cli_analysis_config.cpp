/**
 * @file cli_analysis_config.cpp
 * @brief CLI analysis configuration helpers.
 */

#include "cli_analysis_config.hpp"

namespace scope::cli
{

namespace
{

scope::analysis::AnalysisConfig cliOverridesFromProfileAndFormat(const std::string& profile,
                                                                 const scope::analysis::LogFormat logFormat)
{
    scope::analysis::AnalysisConfig overrides;

    if (!profile.empty())
    {
        if (const std::optional<scope::analysis::FormatProfile> resolved =
                scope::analysis::resolveFormatProfile(profile))
        {
            overrides = resolved->defaults;
        }
    }

    if (logFormat != scope::analysis::LogFormat::Auto)
    {
        overrides.formatHint = logFormat;
    }

    return overrides;
}

} // namespace

scope::analysis::AnalysisConfig buildAnalysisConfig(const AnalyzeOptions& options,
                                                    const configuration::ConfigurationManager& configurationManager)
{
    return scope::analysis::resolveAnalysisConfig(
        configurationManager.configuration(), cliOverridesFromProfileAndFormat(options.profile, options.logFormat));
}

scope::analysis::AnalysisConfig buildAnalysisConfig(
    const InvestigateOptions& options, const configuration::ConfigurationManager& configurationManager)
{
    return scope::analysis::resolveAnalysisConfig(
        configurationManager.configuration(), cliOverridesFromProfileAndFormat(options.profile, options.logFormat));
}

scope::analysis::AnalysisConfig buildAnalysisConfig(const SessionSaveOptions& options,
                                                    const configuration::ConfigurationManager& configurationManager)
{
    return scope::analysis::resolveAnalysisConfig(configurationManager.configuration(),
                                                  cliOverridesFromProfileAndFormat(options.profile,
                                                                                   scope::analysis::LogFormat::Auto));
}

} // namespace scope::cli
