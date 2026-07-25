/**
 * @file ai_investigation_assistant.hpp
 * @brief Orchestrates AI provider calls for investigation (M13).
 */

#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "ai_config.hpp"
#include "ai_provider.hpp"
#include "ai_result.hpp"
#include "foundation/result.hpp"
#include "investigation_result.hpp"
#include "investigation_view.hpp"
#include "nl_query_translator.hpp"
#include "query_node.hpp"

namespace scope::ai
{

/**
 * @brief Holds resolved config and the active AI provider.
 */
class AiInvestigationAssistant
{
  public:
    explicit AiInvestigationAssistant(AiConfig config);

    [[nodiscard]] const AiConfig& config() const noexcept;

    [[nodiscard]] const AiProvider& provider() const noexcept;

    /**
     * @brief Translates natural language to a validated filter DSL expression.
     */
    [[nodiscard]] foundation::Result<std::string> translateNaturalLanguageQuery(
        std::string_view naturalLanguageQuery) const;

    /**
     * @brief Translates natural language to a validated filter query AST.
     */
    [[nodiscard]] foundation::Result<query::QueryNode> translateNaturalLanguageFilter(
        std::string_view naturalLanguageQuery) const;

    /**
     * @brief Produces an investigation summary from bounded context.
     */
    [[nodiscard]] foundation::Result<AiSummary> summarizeInvestigation(
        const investigation::InvestigationView& view,
        const investigation::InvestigationResult& result) const;

  private:
    AiConfig m_config;
    std::unique_ptr<AiProvider> m_provider;
};

} // namespace scope::ai
