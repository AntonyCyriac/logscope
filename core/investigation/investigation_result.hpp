/**
 * @file investigation_result.hpp
 * @brief Content-aware investigation output.
 */

#pragma once

#include <string>
#include <vector>

#include "correlation_summary.hpp"
#include "line_index.hpp"
#include "search_query.hpp"

namespace scope::investigation
{

/**
 * @brief Result of applying investigation criteria to an analysis model.
 */
struct InvestigationResult
{
    std::vector<analysis::IndexedLine> matchingLines;
    CorrelationSummary correlations;
    std::uint64_t indexedLineCount{0U};
    std::uint64_t truncatedLineCount{0U};
    std::string searchQuerySummary;
    search::SearchMode searchMode{search::SearchMode::Text};
};

} // namespace scope::investigation
