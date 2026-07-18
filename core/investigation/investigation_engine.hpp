/**
 * @file investigation_engine.hpp
 * @brief Explores and queries analysis models.
 */

#pragma once

#include <string_view>

#include "analysis_model.hpp"
#include "investigation_view.hpp"
#include "line_count_filter.hpp"

namespace scope::investigation
{

/**
 * @brief Provides search, filter, and inspection over analysis results.
 */
class InvestigationEngine
{
  public:
    /**
     * @brief Creates a detailed inspection view of an analysis model.
     */
    [[nodiscard]] InvestigationView inspect(const analysis::AnalysisModel& model) const;

    /**
     * @brief Determines whether an analysis model satisfies a filter.
     */
    [[nodiscard]] bool matches(const analysis::AnalysisModel& model,
                               const LineCountFilter& filter) const noexcept;

    /**
     * @brief Searches the analyzed source path for a query string.
     */
    [[nodiscard]] bool searchSource(const analysis::AnalysisModel& model,
                                    std::string_view query) const noexcept;
};

} // namespace scope::investigation
