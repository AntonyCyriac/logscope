/**
 * @file desktop_analysis_config.hpp
 * @brief Desktop helpers for AnalysisConfig (storage flags, defaults).
 */

#pragma once

#include "analysis_config.hpp"
#include "configuration_manager.hpp"
#include "report_options.hpp"

namespace scope::desktop
{

[[nodiscard]] inline scope::analysis::AnalysisConfig buildAnalysisConfigForDesktop(
    const scope::configuration::ConfigurationManager& configurationManager, const bool persistIndex,
    const bool reuseIndex, const scope::analysis::LogFormat formatHint = scope::analysis::LogFormat::Auto,
    const std::string& profile = {})
{
    scope::analysis::AnalysisConfig overrides = scope::analysis::AnalysisConfig::defaults();
    overrides.storage.persistIndex = persistIndex;
    overrides.storage.reuseIndex = reuseIndex;

    if (!profile.empty())
    {
        if (const std::optional<scope::analysis::FormatProfile> resolved =
                scope::analysis::resolveFormatProfile(profile))
        {
            overrides = resolved->defaults;
            overrides.storage.persistIndex = persistIndex;
            overrides.storage.reuseIndex = reuseIndex;
        }
    }

    if (formatHint != scope::analysis::LogFormat::Auto)
    {
        overrides.formatHint = formatHint;
    }

    if (persistIndex && overrides.storage.mode == scope::storage::StorageMode::Memory)
    {
        overrides.storage.mode = scope::storage::StorageMode::Hybrid;
    }

    return scope::analysis::resolveAnalysisConfig(configurationManager.configuration(), overrides);
}

[[nodiscard]] inline scope::reporting::ReportOptions defaultReportOptions(
    const scope::configuration::ConfigurationManager& configurationManager)
{
    scope::reporting::ReportOptions options;
    const auto& configuration = configurationManager.configuration();

    if (configuration.has("report.sections"))
    {
        const auto sectionsValue = configuration.get("report.sections");

        if (sectionsValue)
        {
            const auto parsedSections = scope::reporting::ReportSections::parse(*sectionsValue);

            if (parsedSections)
            {
                options.sections = *parsedSections;
            }
        }
    }

    return options;
}

} // namespace scope::desktop
