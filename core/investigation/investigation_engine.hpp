/**
 * @file investigation_engine.hpp
 * @brief Explores and queries analysis models.
 */

#pragma once

#include <string_view>
#include <vector>

#include "analysis_model.hpp"
#include "correlation_summary.hpp"
#include "field_filter.hpp"
#include "investigation_criteria.hpp"
#include "investigation_result.hpp"
#include "investigation_view.hpp"
#include "line_count_filter.hpp"
#include "log_level_filter.hpp"
#include "time_range_filter.hpp"

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
     * @brief Determines whether an analysis model satisfies a line-count filter.
     */
    [[nodiscard]] bool matches(const analysis::AnalysisModel& model,
                               const LineCountFilter& filter) const noexcept;

    /**
     * @brief Determines whether an analysis model satisfies a log-level filter.
     */
    [[nodiscard]] bool matches(const analysis::AnalysisModel& model,
                               const LogLevelFilter& filter) const noexcept;

    /**
     * @brief Searches the analyzed source path for a query string.
     */
    [[nodiscard]] bool searchSource(const analysis::AnalysisModel& model,
                                    std::string_view query) const noexcept;

    /**
     * @brief Searches indexed log line content for a query string.
     */
    [[nodiscard]] std::vector<analysis::IndexedLine> searchContent(const analysis::AnalysisModel& model,
                                                                   std::string_view query) const;

    /**
     * @brief Applies content-aware investigation criteria to indexed lines.
     */
    [[nodiscard]] InvestigationResult investigate(const analysis::AnalysisModel& model,
                                                  const InvestigationCriteria& criteria) const;

    /**
     * @brief Surfaces repeated error messages and shared correlation identifiers.
     */
    [[nodiscard]] CorrelationSummary findCorrelations(const analysis::AnalysisModel& model) const;

  private:
    [[nodiscard]] static bool matchesContentSearch(const analysis::IndexedLine& line,
                                                   std::string_view query) noexcept;

    [[nodiscard]] static bool matchesCriteria(const analysis::IndexedLine& line,
                                              const InvestigationCriteria& criteria) noexcept;
};

} // namespace scope::investigation
