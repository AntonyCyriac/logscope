/**
 * @file ai_config.cpp
 */

#include "ai_config.hpp"

#include <stdexcept>

#include "foundation/error.hpp"
#include "foundation/string.hpp"

namespace scope::ai
{

namespace
{

bool parseBoolean(const std::string& value) noexcept
{
    const std::string normalized = foundation::toLower(foundation::trim(value));

    return normalized == "true" || normalized == "1" || normalized == "yes" || normalized == "on";
}

bool isKnownProvider(const std::string& provider) noexcept
{
    return provider == kProviderNoOp || provider == kProviderHttp;
}

} // namespace

AiConfig resolveAiConfig(const runtime::Configuration& configuration) noexcept
{
    AiConfig config;

    if (configuration.has(kAiEnabledKey))
    {
        const auto enabledResult = configuration.get(kAiEnabledKey);

        if (enabledResult)
        {
            config.enabled = parseBoolean(*enabledResult);
        }
    }

    if (configuration.has(kAiProviderKey))
    {
        const auto providerResult = configuration.get(kAiProviderKey);

        if (providerResult)
        {
            config.provider = foundation::toLower(foundation::trim(*providerResult));
        }
    }

    if (configuration.has(kAiEndpointKey))
    {
        const auto endpointResult = configuration.get(kAiEndpointKey);

        if (endpointResult)
        {
            config.endpoint = foundation::trim(*endpointResult);
        }
    }

    if (configuration.has(kAiModelKey))
    {
        const auto modelResult = configuration.get(kAiModelKey);

        if (modelResult)
        {
            config.model = foundation::trim(*modelResult);
        }
    }

    if (configuration.has(kAiMaxContextLinesKey))
    {
        const auto maxLinesResult = configuration.get(kAiMaxContextLinesKey);

        if (maxLinesResult)
        {
            try
            {
                config.maxContextLines = static_cast<std::uint64_t>(std::stoull(foundation::trim(*maxLinesResult)));
            }
            catch (const std::exception&)
            {
                config.maxContextLines = kDefaultMaxContextLines;
            }
        }
    }

    if (!config.enabled)
    {
        config.provider = kProviderNoOp;
    }

    if (config.provider.empty())
    {
        config.provider = kProviderNoOp;
    }

    return config;
}

foundation::Result<bool> validateAiConfiguration(const runtime::Configuration& configuration) noexcept
{
    std::string provider = kProviderNoOp;

    if (configuration.has(kAiProviderKey))
    {
        const auto providerResult = configuration.get(kAiProviderKey);

        if (!providerResult)
        {
            return foundation::Result<bool>(providerResult.error());
        }

        provider = foundation::toLower(foundation::trim(*providerResult));
    }

    if (!isKnownProvider(provider))
    {
        return foundation::Result<bool>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Unknown ai.provider: " + provider));
    }

    const AiConfig config = resolveAiConfig(configuration);
    if (configuration.has(kAiMaxContextLinesKey))
    {
        const auto maxLinesResult = configuration.get(kAiMaxContextLinesKey);

        if (!maxLinesResult)
        {
            return foundation::Result<bool>(maxLinesResult.error());
        }

        try
        {
            const std::uint64_t maxLines =
                static_cast<std::uint64_t>(std::stoull(foundation::trim(*maxLinesResult)));

            if (maxLines == 0U)
            {
                return foundation::Result<bool>(foundation::Error(
                    foundation::ErrorCode::InvalidArgument, "ai.max_context_lines must be greater than zero."));
            }
        }
        catch (const std::exception&)
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::InvalidArgument, "ai.max_context_lines must be a positive integer."));
        }
    }

    if (config.enabled && config.provider == kProviderHttp)
    {
        if (foundation::isBlank(config.endpoint))
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::InvalidArgument, "ai.endpoint is required when ai.provider=http."));
        }

        if (foundation::isBlank(config.model))
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::InvalidArgument, "ai.model is required when ai.provider=http."));
        }
    }

    return foundation::Result<bool>(true);
}

} // namespace scope::ai
