/**
 * @file ai_context_builder.hpp
 * @brief Builds bounded AI context from investigation output (M13).
 */

#pragma once

#include "ai_config.hpp"
#include "ai_result.hpp"
#include "investigation_result.hpp"
#include "investigation_view.hpp"

namespace scope::ai
{

/**
 * @brief Builds bounded investigation context for provider summarize calls.
 */
[[nodiscard]] AiInvestigationContext buildInvestigationContext(
    const AiConfig& config,
    const investigation::InvestigationView& view,
    const investigation::InvestigationResult& result);

} // namespace scope::ai
