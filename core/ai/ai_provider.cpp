/**
 * @file ai_provider.cpp
 */

#include "ai_provider.hpp"

#include "http_ai_provider.hpp"
#include "noop_ai_provider.hpp"

namespace scope::ai
{

std::unique_ptr<AiProvider> createAiProvider(const AiConfig& config)
{
    if (!config.enabled || config.provider == kProviderNoOp)
    {
        return std::make_unique<NoOpAiProvider>();
    }

    if (config.provider == kProviderHttp)
    {
        return std::make_unique<HttpAiProvider>(config);
    }

    return std::make_unique<NoOpAiProvider>();
}

} // namespace scope::ai
