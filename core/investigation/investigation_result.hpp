/**
 * @file investigation_result.hpp
 * @brief Content-aware investigation output.
 */

#pragma once

#include <vector>

#include "correlation_summary.hpp"
#include "line_index.hpp"

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
};

} // namespace scope::investigation
