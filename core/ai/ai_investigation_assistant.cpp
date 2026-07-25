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

} // namespace scope::ai
