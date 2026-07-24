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

void applyStorageOverrides(scope::analysis::AnalysisConfig& config, const bool persistIndex, const bool reuseIndex,
                           const std::optional<scope::foundation::Path>& indexPath)
{
    config.storage.persistIndex = persistIndex;
    config.storage.reuseIndex = reuseIndex;
    config.storage.indexPath = indexPath;

    if (persistIndex && config.storage.mode == scope::storage::StorageMode::Memory)
    {
        config.storage.mode = scope::storage::StorageMode::Hybrid;
    }
}

} // namespace

scope::analysis::AnalysisConfig buildAnalysisConfig(const AnalyzeOptions& options,
                                                    const configuration::ConfigurationManager& configurationManager)
{
    scope::analysis::AnalysisConfig config = scope::analysis::resolveAnalysisConfig(
        configurationManager.configuration(), cliOverridesFromProfileAndFormat(options.profile, options.logFormat));
    applyStorageOverrides(config, options.persistIndex, options.reuseIndex, options.indexPath);

    return config;
}

scope::analysis::AnalysisConfig buildAnalysisConfig(
    const InvestigateOptions& options, const configuration::ConfigurationManager& configurationManager)
{
    scope::analysis::AnalysisConfig config = scope::analysis::resolveAnalysisConfig(
        configurationManager.configuration(), cliOverridesFromProfileAndFormat(options.profile, options.logFormat));
    applyStorageOverrides(config, options.persistIndex, options.reuseIndex, options.indexPath);

    return config;
}

scope::analysis::AnalysisConfig buildAnalysisConfig(const SessionSaveOptions& options,
                                                    const configuration::ConfigurationManager& configurationManager)
{
    scope::analysis::AnalysisConfig config = scope::analysis::resolveAnalysisConfig(
        configurationManager.configuration(),
        cliOverridesFromProfileAndFormat(options.profile, scope::analysis::LogFormat::Auto));
    applyStorageOverrides(config, options.persistIndex, options.reuseIndex, options.indexPath);

    return config;
}

scope::analysis::AnalysisConfig buildAnalysisConfig(const AnalyticsOptions& options,
                                                    const configuration::ConfigurationManager& configurationManager)
{
    return scope::analysis::resolveAnalysisConfig(
        configurationManager.configuration(), cliOverridesFromProfileAndFormat(options.profile, options.logFormat));
}

} // namespace scope::cli
