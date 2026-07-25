/**
 * @file http_ai_provider.hpp
 * @brief HTTP / OpenAI-compatible AI provider (M13.6 stub in M13.1).
 */

#pragma once

#include "ai_config.hpp"
#include "ai_provider.hpp"

namespace scope::ai
{

/**
 * @brief HTTP-backed provider; full implementation in M13.6.
 */
class HttpAiProvider final : public AiProvider
{
  public:
    explicit HttpAiProvider(AiConfig config);

    [[nodiscard]] std::string id() const override;

    [[nodiscard]] foundation::Result<std::string>
    translateNlToFilter(std::string_view naturalLanguageQuery) const override;

    [[nodiscard]] foundation::Result<AiSummary> summarize(const AiInvestigationContext& context) const override;

    [[nodiscard]] foundation::Result<std::vector<AiAnomalyHint>>
    suggestAnomalies(const AiAnalyticsContext& context) const override;

  private:
    AiConfig m_config;
};

} // namespace scope::ai
