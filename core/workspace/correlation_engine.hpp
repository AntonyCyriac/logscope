/**
 * @file correlation_engine.hpp
 * @brief Correlation suggestion engine (Story 6 / v2.8.0).
 */

#pragma once

#include <string>
#include <unordered_set>
#include <vector>

#include "correlation_suggestion.hpp"
#include "evidence_link.hpp"
#include "timeline_event.hpp"

namespace scope::workspace
{

/**
 * @brief Computes ephemeral correlation suggestions from timeline events.
 */
class CorrelationEngine
{
  public:
    [[nodiscard]] static CorrelationSuggestionListResult computeSuggestions(
        const std::string& investigationId, const std::vector<TimelineEvent>& events,
        const std::vector<EvidenceLink>& links, const std::unordered_set<std::string>& dismissedSuggestionIds,
        CorrelationSuggestionQuery query);
};

} // namespace scope::workspace
