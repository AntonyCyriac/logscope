/**
 * @file analysis_config.cpp
 * @brief Analysis configuration resolution and validation.
 */

#include "analysis_config.hpp"

#include <charconv>
#include <vector>

#include "foundation/error.hpp"
#include "foundation/string.hpp"

namespace scope::analysis
{

namespace
{

constexpr std::size_t minConfigurableIndexedLines = 1U;

bool parsePositiveSize(std::string_view value, std::size_t& output) noexcept
{
    if (value.empty())
    {
        return false;
    }

    std::uint64_t parsed = 0U;
    const auto result = std::from_chars(value.data(), value.data() + value.size(), parsed);

    if (result.ec != std::errc{} || result.ptr != value.data() + value.size() || parsed == 0U)
    {
        return false;
    }

    output = static_cast<std::size_t>(parsed);

    return true;
}

void applyConfigValue(const std::string& key, const std::string& value, AnalysisConfig& config)
{
    if (key == "source.format")
    {
        const auto parsed = parseLogFormat(value);

        if (parsed && *parsed != LogFormat::Unknown)
        {
            config.formatHint = *parsed;
        }

        return;
    }

    if (key == "source.json.timestamp_field")
    {
        config.jsonFieldMapping.timestampField = value;

        return;
    }

    if (key == "source.json.level_field")
    {
        config.jsonFieldMapping.levelField = value;

        return;
    }

    if (key == "investigation.max_indexed_lines")
    {
        std::size_t parsed = 0U;

        if (parsePositiveSize(value, parsed))
        {
            config.maxIndexedLines = parsed;
        }
    }
}

void applyConfiguration(const runtime::Configuration& configuration, AnalysisConfig& config)
{
    const std::vector<std::string> keys = {"profile",
                                           "source.format",
                                           "source.json.timestamp_field",
                                           "source.json.level_field",
                                           "investigation.max_indexed_lines"};

    if (configuration.has("profile"))
    {
        const auto profileValue = configuration.get("profile");

        if (profileValue)
        {
            if (const std::optional<FormatProfile> profile = resolveFormatProfile(*profileValue))
            {
                config = profile->defaults;
            }
        }
    }

    for (const std::string& key : keys)
    {
        if (key == "profile")
        {
            continue;
        }

        if (configuration.has(key))
        {
            const auto value = configuration.get(key);

            if (value)
            {
                applyConfigValue(key, *value, config);
            }
        }
    }
}

void mergeCliOverrides(const AnalysisConfig& cliOverrides, AnalysisConfig& config)
{
    if (cliOverrides.formatHint != LogFormat::Auto)
    {
        config.formatHint = cliOverrides.formatHint;
    }

    if (cliOverrides.jsonFieldMapping.hasTimestampOverride())
    {
        config.jsonFieldMapping.timestampField = cliOverrides.jsonFieldMapping.timestampField;
    }

    if (cliOverrides.jsonFieldMapping.hasLevelOverride())
    {
        config.jsonFieldMapping.levelField = cliOverrides.jsonFieldMapping.levelField;
    }

    if (cliOverrides.maxIndexedLines != defaultIndexedLineCapacity)
    {
        config.maxIndexedLines = cliOverrides.maxIndexedLines;
    }
}

} // namespace

AnalysisConfig AnalysisConfig::defaults() noexcept
{
    return AnalysisConfig{};
}

std::optional<FormatProfile> resolveFormatProfile(const std::string_view name) noexcept
{
    const std::string normalized = foundation::toLower(name);

    if (normalized == "generic-plain")
    {
        FormatProfile profile;
        profile.name = "generic-plain";
        profile.defaults.formatHint = LogFormat::PlainText;

        return profile;
    }

    if (normalized == "generic-json")
    {
        FormatProfile profile;
        profile.name = "generic-json";
        profile.defaults.formatHint = LogFormat::JsonLines;

        return profile;
    }

    return std::nullopt;
}

std::string supportedProfilesList() noexcept
{
    return "generic-plain, generic-json";
}

AnalysisConfig resolveAnalysisConfig(const runtime::Configuration& configuration,
                                     const AnalysisConfig& cliOverrides) noexcept
{
    AnalysisConfig config = AnalysisConfig::defaults();
    applyConfiguration(configuration, config);
    mergeCliOverrides(cliOverrides, config);

    if (config.maxIndexedLines > maxConfigurableIndexedLines)
    {
        config.maxIndexedLines = maxConfigurableIndexedLines;
    }

    if (config.maxIndexedLines < minConfigurableIndexedLines)
    {
        config.maxIndexedLines = minConfigurableIndexedLines;
    }

    config.storage = storage::resolveStorageConfig(configuration, cliOverrides.storage);

    return config;
}

foundation::Result<bool> validateAnalysisConfiguration(const runtime::Configuration& configuration) noexcept
{
    if (configuration.has("profile"))
    {
        const auto profileValue = configuration.get("profile");

        if (!profileValue || profileValue->empty())
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::InvalidArgument, "Configuration key profile must not be empty."));
        }

        if (!resolveFormatProfile(*profileValue))
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::InvalidArgument,
                "Unknown profile '" + *profileValue + "'. Supported profiles: " + supportedProfilesList() + "."));
        }
    }

    if (configuration.has("source.format"))
    {
        const auto formatValue = configuration.get("source.format");

        if (!formatValue || formatValue->empty())
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::InvalidArgument, "Configuration key source.format must not be empty."));
        }

        const auto parsed = parseLogFormat(*formatValue);

        if (!parsed || *parsed == LogFormat::Unknown)
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::InvalidArgument,
                "Invalid source.format value. Use auto, plain, or jsonl."));
        }
    }

    if (configuration.has("source.json.timestamp_field"))
    {
        const auto fieldValue = configuration.get("source.json.timestamp_field");

        if (!fieldValue || foundation::isBlank(*fieldValue))
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::InvalidArgument,
                "Configuration key source.json.timestamp_field must not be empty."));
        }
    }

    if (configuration.has("source.json.level_field"))
    {
        const auto fieldValue = configuration.get("source.json.level_field");

        if (!fieldValue || foundation::isBlank(*fieldValue))
        {
            return foundation::Result<bool>(
                foundation::Error(foundation::ErrorCode::InvalidArgument,
                                    "Configuration key source.json.level_field must not be empty."));
        }
    }

    if (configuration.has("investigation.max_indexed_lines"))
    {
        const auto limitValue = configuration.get("investigation.max_indexed_lines");

        if (!limitValue)
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::InvalidArgument,
                "Configuration key investigation.max_indexed_lines must not be empty."));
        }

        std::size_t parsed = 0U;

        if (!parsePositiveSize(*limitValue, parsed) || parsed > maxConfigurableIndexedLines)
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::InvalidArgument,
                "Invalid investigation.max_indexed_lines. Use a positive integer up to " +
                    std::to_string(maxConfigurableIndexedLines) + "."));
        }
    }

    return foundation::Result<bool>(true);
}

} // namespace scope::analysis
