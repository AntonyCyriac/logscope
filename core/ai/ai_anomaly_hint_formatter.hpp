/**
 * @file ai_anomaly_hint_formatter.hpp
 * @brief Renders structured anomaly hints for CLI output (M13).
 */

#pragma once

#include <string>
#include <vector>

#include "ai_result.hpp"

namespace scope::ai
{

/**
 * @brief Formats anomaly hints into human-readable sections.
 */
[[nodiscard]] std::string formatAiAnomalyHints(const std::vector<AiAnomalyHint>& hints);

} // namespace scope::ai
