/**
 * @file noop_ai_provider.hpp
 * @brief Deterministic offline AI provider (M13).
 */

#pragma once

#include "ai_provider.hpp"

namespace scope::ai
{

/**
 * @brief Rule-based provider for CI and ai.enabled=false paths.
 */
class NoOpAiProvider final : public AiProvider
{
  public:
    [[nodiscard]] std::string id() const override;

    [[nodiscard]] foundation::Result<std::string>
    translateNlToFilter(std::string_view naturalLanguageQuery) const override;

    [[nodiscard]] foundation::Result<AiSummary> summarize(const AiInvestigationContext& context) const override;

    [[nodiscard]] foundation::Result<std::vector<AiAnomalyHint>>
    suggestAnomalies(const AiAnalyticsContext& context) const override;
};

} // namespace scope::ai
