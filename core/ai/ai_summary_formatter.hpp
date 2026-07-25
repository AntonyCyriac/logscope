/**
 * @file ai_summary_formatter.hpp
 * @brief Renders structured AI summaries for CLI output (M13).
 */

#pragma once

#include <string>

#include "ai_result.hpp"

namespace scope::ai
{

/**
 * @brief Formats an AI summary into human-readable sections.
 */
[[nodiscard]] std::string formatAiSummary(const AiSummary& summary);

} // namespace scope::ai
