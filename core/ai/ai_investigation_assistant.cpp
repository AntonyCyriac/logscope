/**
 * @file ai_investigation_assistant.cpp
 */

#include "ai_investigation_assistant.hpp"

#include "ai_analytics_context_builder.hpp"
#include "ai_context_builder.hpp"
#include "foundation/error.hpp"

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

foundation::Result<AiSummary> AiInvestigationAssistant::summarizeInvestigation(
    const investigation::InvestigationView& view,
    const investigation::InvestigationResult& result) const
{
    if (!m_config.enabled)
    {
        return foundation::Result<AiSummary>(foundation::Error(
            foundation::ErrorCode::InvalidArgument,
            "AI assistant is disabled (ai.enabled=false)."));
    }

    const AiInvestigationContext context = buildInvestigationContext(m_config, view, result);

    return m_provider->summarize(context);
}

foundation::Result<std::vector<AiAnomalyHint>> AiInvestigationAssistant::suggestAnomalyHints(
    const analytics::AnalyticsResult& analytics) const
{
    if (!m_config.enabled)
    {
        return foundation::Result<std::vector<AiAnomalyHint>>(foundation::Error(
            foundation::ErrorCode::InvalidArgument,
            "AI assistant is disabled (ai.enabled=false)."));
    }

    const AiAnalyticsContext context = buildAnalyticsContext(analytics);

    return m_provider->suggestAnomalies(context);
}

} // namespace scope::ai
