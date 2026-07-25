/**
 * @file ai_provider.hpp
 * @brief AI provider interface (M13).
 */

#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "ai_config.hpp"
#include "ai_result.hpp"
#include "foundation/result.hpp"

namespace scope::ai
{

/**
 * @brief Pluggable AI backend for investigation assistance.
 */
class AiProvider
{
  public:
    virtual ~AiProvider() = default;

    [[nodiscard]] virtual std::string id() const = 0;

    [[nodiscard]] virtual foundation::Result<std::string>
    translateNlToFilter(std::string_view naturalLanguageQuery) const = 0;

    [[nodiscard]] virtual foundation::Result<AiSummary>
    summarize(const AiInvestigationContext& context) const = 0;

    [[nodiscard]] virtual foundation::Result<std::vector<AiAnomalyHint>>
    suggestAnomalies(const AiAnalyticsContext& context) const = 0;
};

[[nodiscard]] std::unique_ptr<AiProvider> createAiProvider(const AiConfig& config);

} // namespace scope::ai
