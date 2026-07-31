/**
 * @file web_config.cpp
 */

#include "web_config.hpp"

#include <cstdlib>

#include <algorithm>

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

void addOriginIfMissing(std::vector<std::string>& origins, const std::string& origin)
{
    if (std::find(origins.begin(), origins.end(), origin) == origins.end())
    {
        origins.push_back(origin);
    }
}

void addSchemeOrigins(std::vector<std::string>& origins, const char* scheme, const std::string& host, const int port)
{
    addOriginIfMissing(origins, std::string(scheme) + host + ":" + std::to_string(port));
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
        corsOriginsUserSet = true;
    }

    if (const auto value = configuration.get("web.tls_cert"))
    {
        tlsCertPath = foundation::Path(value.value());
    }

    if (const auto value = configuration.get("web.tls_key"))
    {
        tlsKeyPath = foundation::Path(value.value());
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

    if (const auto value = configuration.get("web.workspace_dir"))
    {
        workspaceDir = foundation::Path(value.value());
        workspaceDirUserSet = true;
    }

    if (const auto value = configuration.get("web.workspaces_list_limit"))
    {
        workspacesListLimit = parseInt(value.value(), workspacesListLimit);
    }

    if (const auto value = configuration.get("web.async_analyze_threshold_bytes"))
    {
        asyncAnalyzeThresholdBytes = parseByteSize(value.value(), asyncAnalyzeThresholdBytes);
    }

    if (const auto value = configuration.get("web.job_ttl_seconds"))
    {
        jobTtlSeconds = parseInt(value.value(), jobTtlSeconds);
    }

    if (const auto value = configuration.get("web.job_max_concurrent_per_session"))
    {
        jobMaxConcurrentPerSession = parseInt(value.value(), jobMaxConcurrentPerSession);
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

    if (const char* value = std::getenv("LOGSCOPE_WEB_TLS_CERT"))
    {
        tlsCertPath = foundation::Path(value);
    }

    if (const char* value = std::getenv("LOGSCOPE_WEB_TLS_KEY"))
    {
        tlsKeyPath = foundation::Path(value);
    }

    if (const char* value = std::getenv("LOGSCOPE_WEB_WORKSPACE_DIR"))
    {
        workspaceDir = foundation::Path(value);
        workspaceDirUserSet = true;
    }

    if (const char* value = std::getenv("LOGSCOPE_WEB_WORKSPACES_LIST_LIMIT"))
    {
        workspacesListLimit = parseInt(value, workspacesListLimit);
    }

    if (const char* value = std::getenv("LOGSCOPE_WEB_ASYNC_ANALYZE_THRESHOLD_BYTES"))
    {
        asyncAnalyzeThresholdBytes = parseByteSize(value, asyncAnalyzeThresholdBytes);
    }

    if (const char* value = std::getenv("LOGSCOPE_WEB_JOB_TTL_SECONDS"))
    {
        jobTtlSeconds = parseInt(value, jobTtlSeconds);
    }

    if (const char* value = std::getenv("LOGSCOPE_WEB_JOB_MAX_CONCURRENT_PER_SESSION"))
    {
        jobMaxConcurrentPerSession = parseInt(value, jobMaxConcurrentPerSession);
    }
}

void WebConfig::applyDerivedDefaults()
{
    if (corsOriginsUserSet || bindPort == 0)
    {
        return;
    }

    std::vector<std::string> derived;
    addSchemeOrigins(derived, "http://", bindHost, bindPort);

    if (tlsEnabled())
    {
        addSchemeOrigins(derived, "https://", bindHost, bindPort);
    }

    if (bindHost == "127.0.0.1" || bindHost == "0.0.0.0" || bindHost == "localhost")
    {
        addSchemeOrigins(derived, "http://", "localhost", bindPort);
        addSchemeOrigins(derived, "http://", "127.0.0.1", bindPort);

        if (tlsEnabled())
        {
            addSchemeOrigins(derived, "https://", "localhost", bindPort);
            addSchemeOrigins(derived, "https://", "127.0.0.1", bindPort);
        }
    }

    corsOrigins = std::move(derived);
}

bool WebConfig::tlsEnabled() const
{
    return !tlsCertPath.isEmpty() && !tlsKeyPath.isEmpty();
}

const char* WebConfig::urlScheme() const noexcept
{
    return tlsEnabled() ? "https" : "http";
}

std::string WebConfig::listenUrl() const
{
    return std::string(urlScheme()) + "://" + bindHost + ':' + std::to_string(bindPort);
}

} // namespace scope::web
