/**
 * @file investigation_criteria.hpp
 * @brief Combined content-aware investigation filters.
 */

#pragma once

#include <string>

#include "field_filter.hpp"
#include "time_range_filter.hpp"

namespace scope::investigation
{

/**
 * @brief User-selected content investigation filters.
 */
struct InvestigationCriteria
{
    std::string contentSearch;
    TimeRangeFilter timeRange = TimeRangeFilter::any();
    FieldFilter field = FieldFilter::any();

    [[nodiscard]] bool isActive() const noexcept;
};

} // namespace scope::investigation
