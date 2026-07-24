/**
 * @file investigation_criteria.hpp
 * @brief Combined content-aware investigation filters.
 */

#pragma once

#include <optional>
#include <string>

#include "field_filter.hpp"
#include "foundation/result.hpp"
#include "query_node.hpp"
#include "search_query.hpp"
#include "time_range_filter.hpp"

namespace scope::investigation
{

/**
 * @brief User-selected content investigation filters.
 */
struct InvestigationCriteria
{
    std::string contentSearch;
    std::string booleanQuery;
    std::string filterExpression;
    std::optional<search::SearchQuery> searchQuery;
    std::optional<query::QueryNode> filterQuery;
    search::SearchMode searchMode = search::SearchMode::Text;
    search::CaseSensitivity caseSensitivity = search::CaseSensitivity::Insensitive;

    TimeRangeFilter timeRange = TimeRangeFilter::any();
    FieldFilter field = FieldFilter::any();

    [[nodiscard]] bool isActive() const noexcept;

    /**
     * @brief Resolves the effective search query from explicit and legacy fields.
     */
    [[nodiscard]] foundation::Result<search::SearchQuery> resolvedSearchQuery() const noexcept;

    /**
     * @brief Resolves the effective filter query from explicit and legacy fields.
     */
    [[nodiscard]] foundation::Result<query::QueryNode> resolvedFilterQuery() const noexcept;
};

} // namespace scope::investigation
