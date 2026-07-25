/**
 * @file ai_investigation_assistant.hpp
 * @brief Orchestrates AI provider calls for investigation (M13).
 */

#pragma once

#include <memory>

#include "ai_config.hpp"
#include "ai_provider.hpp"

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

  private:
    AiConfig m_config;
    std::unique_ptr<AiProvider> m_provider;
};

} // namespace scope::ai
