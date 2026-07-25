/**
 * @file ai_analytics_context_builder.hpp
 * @brief Builds bounded AI context from analytics output (M13).
 */

#pragma once

#include "ai_result.hpp"
#include "analytics_result.hpp"

namespace scope::ai
{

/**
 * @brief Builds bounded analytics context for provider anomaly-hint calls.
 */
[[nodiscard]] AiAnalyticsContext buildAnalyticsContext(const analytics::AnalyticsResult& analytics);

} // namespace scope::ai
