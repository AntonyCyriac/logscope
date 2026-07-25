/**
 * @file nl_query_translator.hpp
 * @brief Natural-language to filter DSL translation with validation (M13).
 */

#pragma once

#include <string>
#include <string_view>

#include "ai_provider.hpp"
#include "foundation/result.hpp"
#include "query_node.hpp"

namespace scope::ai
{

/**
 * @brief Translates natural language to validated filter DSL via an AI provider.
 */
class NlQueryTranslator
{
  public:
    explicit NlQueryTranslator(const AiProvider& provider) noexcept;

    /**
     * @brief Returns a validated filter DSL expression string.
     */
    [[nodiscard]] foundation::Result<std::string> translateToFilterExpression(
        std::string_view naturalLanguageQuery) const;

    /**
     * @brief Returns a parsed and validated filter query AST.
     */
    [[nodiscard]] foundation::Result<query::QueryNode> translateToFilterQuery(
        std::string_view naturalLanguageQuery) const;

  private:
    const AiProvider& m_provider;
};

} // namespace scope::ai
