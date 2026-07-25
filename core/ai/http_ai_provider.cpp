/**
 * @file http_ai_provider.cpp
 */

#include "http_ai_provider.hpp"

#include <cstdlib>

#include "foundation/error.hpp"
#include "foundation/string.hpp"

namespace scope::ai
{

namespace
{

foundation::Error notImplementedError()
{
    return foundation::Error(foundation::ErrorCode::Unknown,
                             "HTTP AI provider is not implemented until M13.6.");
}

foundation::Error missingApiKeyError()
{
    return foundation::Error(foundation::ErrorCode::InvalidArgument,
                             "LOGSCOPE_AI_API_KEY is required when ai.provider=http.");
}

bool hasApiKey()
{
    if (const char* apiKey = std::getenv("LOGSCOPE_AI_API_KEY"))
    {
        return !foundation::isBlank(apiKey);
    }

    return false;
}

} // namespace

HttpAiProvider::HttpAiProvider(AiConfig config) : m_config(std::move(config)) {}

std::string HttpAiProvider::id() const
{
    return kProviderHttp;
}

foundation::Result<std::string> HttpAiProvider::translateNlToFilter(std::string_view /*naturalLanguageQuery*/) const
{
    if (!hasApiKey())
    {
        return foundation::Result<std::string>(missingApiKeyError());
    }

    return foundation::Result<std::string>(notImplementedError());
}

foundation::Result<AiSummary> HttpAiProvider::summarize(const AiInvestigationContext& /*context*/) const
{
    if (!hasApiKey())
    {
        return foundation::Result<AiSummary>(missingApiKeyError());
    }

    return foundation::Result<AiSummary>(notImplementedError());
}

foundation::Result<std::vector<AiAnomalyHint>> HttpAiProvider::suggestAnomalies(
    const AiAnalyticsContext& /*context*/) const
{
    if (!hasApiKey())
    {
        return foundation::Result<std::vector<AiAnomalyHint>>(missingApiKeyError());
    }

    return foundation::Result<std::vector<AiAnomalyHint>>(notImplementedError());
}

} // namespace scope::ai
