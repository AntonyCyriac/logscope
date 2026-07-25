/**
 * @file http_ai_client.cpp
 */

#include "http_ai_client.hpp"

#include <cstdlib>

#include "ai_json_util.hpp"
#include "foundation/error.hpp"
#include "foundation/string.hpp"

#include <httplib.h>

namespace scope::ai
{

namespace
{

struct EndpointParts
{
    std::string baseUrl;
    std::string apiPrefix;
};

foundation::Error missingApiKeyError()
{
    return foundation::Error(foundation::ErrorCode::InvalidArgument,
                             "LOGSCOPE_AI_API_KEY is required when ai.provider=http.");
}

std::string apiKey()
{
    if (const char* value = std::getenv("LOGSCOPE_AI_API_KEY"))
    {
        return std::string(value);
    }

    return std::string();
}

std::string normalizeEndpoint(std::string endpoint)
{
    while (!endpoint.empty() && endpoint.back() == '/')
    {
        endpoint.pop_back();
    }

    return endpoint;
}

EndpointParts parseEndpoint(const std::string& endpoint)
{
    EndpointParts parts;
    const std::string normalized = normalizeEndpoint(endpoint);
    const std::size_t schemeEnd = normalized.find("://");

    if (schemeEnd == std::string::npos)
    {
        parts.baseUrl = normalized;
        parts.apiPrefix = "/v1";

        return parts;
    }

    const std::size_t pathStart = normalized.find('/', schemeEnd + 3U);

    if (pathStart == std::string::npos)
    {
        parts.baseUrl = normalized;
        parts.apiPrefix = "/v1";

        return parts;
    }

    parts.baseUrl = normalized.substr(0U, pathStart);
    parts.apiPrefix = normalized.substr(pathStart);

    return parts;
}

} // namespace

HttpAiClient::HttpAiClient(AiConfig config) : m_config(std::move(config)), m_settings{10, 60} {}

HttpAiClient::HttpAiClient(AiConfig config, Settings settings)
    : m_config(std::move(config)), m_settings(settings)
{
}

foundation::Result<std::string> HttpAiClient::chatCompletion(const std::string_view systemPrompt,
                                                             const std::string_view userPrompt) const
{
    const std::string key = apiKey();

    if (foundation::isBlank(key))
    {
        return foundation::Result<std::string>(missingApiKeyError());
    }

    if (foundation::isBlank(m_config.endpoint))
    {
        return foundation::Result<std::string>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "ai.endpoint is required when ai.provider=http."));
    }

    if (foundation::isBlank(m_config.model))
    {
        return foundation::Result<std::string>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "ai.model is required when ai.provider=http."));
    }

    const EndpointParts endpoint = parseEndpoint(m_config.endpoint);
    const std::string requestBody = buildChatCompletionRequest(m_config.model, systemPrompt, userPrompt);
    const std::string path = endpoint.apiPrefix + "/chat/completions";

    httplib::Client client(endpoint.baseUrl.c_str());
    client.set_connection_timeout(m_settings.connectTimeoutSeconds, 0);
    client.set_read_timeout(m_settings.readTimeoutSeconds, 0);

    httplib::Headers headers = {{"Authorization", "Bearer " + key}, {"Content-Type", "application/json"}};

    const auto response = client.Post(path.c_str(), headers, requestBody, "application/json");

    if (!response)
    {
        return foundation::Result<std::string>(foundation::Error(
            foundation::ErrorCode::IOError, "HTTP AI request failed: unable to reach " + m_config.endpoint + "."));
    }

    if (response->status < 200 || response->status >= 300)
    {
        return foundation::Result<std::string>(foundation::Error(
            foundation::ErrorCode::IOError,
            "HTTP AI request failed with status " + std::to_string(response->status) + ": " + response->body + "."));
    }

    const auto content = extractChatCompletionContent(response->body);

    if (!content)
    {
        return foundation::Result<std::string>(foundation::Error(
            foundation::ErrorCode::ParseError, "HTTP AI response did not contain assistant content."));
    }

    return foundation::Result<std::string>(foundation::trim(*content));
}

} // namespace scope::ai
