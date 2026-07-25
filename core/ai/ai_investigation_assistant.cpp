/**
 * @file ai_investigation_assistant.cpp
 */

#include "ai_investigation_assistant.hpp"

namespace scope::ai
{

AiInvestigationAssistant::AiInvestigationAssistant(AiConfig config)
    : m_config(std::move(config)), m_provider(createAiProvider(m_config))
{
}

const AiConfig& AiInvestigationAssistant::config() const noexcept
{
    return m_config;
}

const AiProvider& AiInvestigationAssistant::provider() const noexcept
{
    return *m_provider;
}

foundation::Result<std::string> AiInvestigationAssistant::translateNaturalLanguageQuery(
    std::string_view naturalLanguageQuery) const
{
    const NlQueryTranslator translator(*m_provider);

    return translator.translateToFilterExpression(naturalLanguageQuery);
}

foundation::Result<query::QueryNode> AiInvestigationAssistant::translateNaturalLanguageFilter(
    std::string_view naturalLanguageQuery) const
{
    const NlQueryTranslator translator(*m_provider);

    return translator.translateToFilterQuery(naturalLanguageQuery);
}

} // namespace scope::ai
