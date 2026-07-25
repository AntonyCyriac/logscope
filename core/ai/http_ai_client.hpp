/**
 * @file http_ai_client.hpp
 * @brief OpenAI-compatible chat completion HTTP client (M13).
 */

#pragma once

#include <string>
#include <string_view>

#include "ai_config.hpp"
#include "foundation/result.hpp"

namespace scope::ai
{

/**
 * @brief Minimal HTTP client for OpenAI-compatible chat completions.
 */
class HttpAiClient
{
  public:
    struct Settings
    {
        int connectTimeoutSeconds;
        int readTimeoutSeconds;
    };

    explicit HttpAiClient(AiConfig config);

    HttpAiClient(AiConfig config, Settings settings);

    [[nodiscard]] foundation::Result<std::string> chatCompletion(std::string_view systemPrompt,
                                                                 std::string_view userPrompt) const;

  private:
    AiConfig m_config;
    Settings m_settings;
};

} // namespace scope::ai
