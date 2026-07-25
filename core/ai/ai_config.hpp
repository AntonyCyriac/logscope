/**
 * @file ai_config.hpp
 * @brief AI assistant configuration (M13).
 */

#pragma once

#include <cstdint>
#include <string>

#include "foundation/result.hpp"
#include "runtime/configuration.hpp"

namespace scope::ai
{

constexpr const char* kAiEnabledKey = "ai.enabled";
constexpr const char* kAiProviderKey = "ai.provider";
constexpr const char* kAiEndpointKey = "ai.endpoint";
constexpr const char* kAiModelKey = "ai.model";
constexpr const char* kAiMaxContextLinesKey = "ai.max_context_lines";

constexpr const char* kProviderNoOp = "noop";
constexpr const char* kProviderHttp = "http";

constexpr std::uint64_t kDefaultMaxContextLines = 200U;

/**
 * @brief Resolved AI configuration.
 */
struct AiConfig
{
    bool enabled{false};
    std::string provider{kProviderNoOp};
    std::string endpoint;
    std::string model;
    std::uint64_t maxContextLines{kDefaultMaxContextLines};
};

[[nodiscard]] AiConfig resolveAiConfig(const runtime::Configuration& configuration) noexcept;

[[nodiscard]] foundation::Result<bool> validateAiConfiguration(
    const runtime::Configuration& configuration) noexcept;

} // namespace scope::ai
