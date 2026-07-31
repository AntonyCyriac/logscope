/**
 * @file web_config.cpp
 */

#include "web_config.hpp"

#include <cstdlib>

#include "foundation/string.hpp"

namespace scope::web
{

namespace
{

std::uint64_t parseByteSize(const std::string& value, const std::uint64_t fallback)
{
    if (value.empty())
    {
        return fallback;
    }

    try
    {
        return static_cast<std::uint64_t>(std::stoull(value));
    }
    catch (...)
    {
        return fallback;
    }
}

int parseInt(const std::string& value, const int fallback)
{
    if (value.empty())
    {
        return fallback;
    }

    try
    {
        return std::stoi(value);
    }
    catch (...)
    {
        return fallback;
    }
}

std::vector<std::string> splitCommaList(const std::string& value)
{
    std::vector<std::string> items;
    std::string current;

    for (const char character : value)
    {
        if (character == ',')
        {
            if (!current.empty())
            {
                items.push_back(scope::foundation::trim(current));
                current.clear();
            }

            continue;
        }

        current.push_back(character);
    }

    if (!current.empty())
    {
        items.push_back(scope::foundation::trim(current));
    }

    return items;
}

} // namespace

WebConfig WebConfig::defaults()
{
    return WebConfig{};
}

void WebConfig::mergeFromConfiguration(const configuration::ConfigurationManager& configurationManager)
{
    const runtime::Configuration& configuration = configurationManager.configuration();

    if (const auto value = configuration.get("web.bind_host"))
    {
        bindHost = value.value();
    }

    if (const auto value = configuration.get("web.bind_port"))
    {
        bindPort = parseInt(value.value(), bindPort);
    }

    if (const auto value = configuration.get("web.api_key"))
    {
        apiKey = value.value();
    }

    if (const auto value = configuration.get("web.cors_origins"))
    {
        corsOrigins = splitCommaList(value.value());
    }

    if (const auto value = configuration.get("web.max_upload_bytes"))
    {
        maxUploadBytes = parseByteSize(value.value(), maxUploadBytes);
    }

    if (const auto value = configuration.get("web.upload_temp_dir"))
    {
        uploadTempDir = foundation::Path(value.value());
    }

    if (const auto value = configuration.get("web.allow_server_paths"))
    {
        const std::string normalized = scope::foundation::trim(value.value());
        allowServerPaths = normalized == "true" || normalized == "1" || normalized == "TRUE";
    }

    if (const auto value = configuration.get("web.allowed_path_roots"))
    {
        allowedPathRoots.clear();

        for (const std::string& root : splitCommaList(value.value()))
        {
            allowedPathRoots.emplace_back(root);
        }
    }

    if (const auto value = configuration.get("web.request_timeout_seconds"))
    {
        requestTimeoutSeconds = parseInt(value.value(), requestTimeoutSeconds);
    }
}

void WebConfig::applyEnvironment()
{
    if (const char* value = std::getenv("LOGSCOPE_WEB_API_KEY"))
    {
        apiKey = value;
    }

    if (const char* value = std::getenv("LOGSCOPE_WEB_BIND_HOST"))
    {
        bindHost = value;
    }

    if (const char* value = std::getenv("LOGSCOPE_WEB_BIND_PORT"))
    {
        bindPort = parseInt(value, bindPort);
    }
}

} // namespace scope::web
