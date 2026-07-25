/**
 * @file ai_json_util.hpp
 * @brief Minimal JSON helpers for OpenAI-compatible HTTP responses (M13).
 */

#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace scope::ai
{

[[nodiscard]] std::string escapeJsonString(std::string_view value);

[[nodiscard]] std::string buildChatCompletionRequest(std::string_view model, std::string_view systemPrompt,
                                                     std::string_view userPrompt);

[[nodiscard]] std::optional<std::string> extractChatCompletionContent(std::string_view responseBody);

} // namespace scope::ai
