/**
 * @file line_count_filter.hpp
 * @brief Filters analysis models by total line count.
 */

#pragma once

#include <cstdint>
#include <limits>

#include "analysis_model.hpp"

namespace scope::investigation
{

/**
 * @brief Progressive line-count filter for analysis models.
 */
class LineCountFilter
{
  public:
    /**
     * @brief Creates a filter that matches any line count.
     */
    [[nodiscard]] static LineCountFilter any() noexcept;

    /**
     * @brief Creates a filter that matches models with at least one line.
     */
    [[nodiscard]] static LineCountFilter nonEmpty() noexcept;

    /**
     * @brief Returns a copy with a raised minimum line count.
     */
    [[nodiscard]] LineCountFilter withMinimum(std::uint64_t minLines) const noexcept;

    /**
     * @brief Returns a copy with a lowered maximum line count.
     */
    [[nodiscard]] LineCountFilter withMaximum(std::uint64_t maxLines) const noexcept;

    /**
     * @brief Determines whether the model satisfies this filter.
     */
    [[nodiscard]] bool matches(const analysis::AnalysisModel& model) const noexcept;

  private:
    std::uint64_t m_minLines{0U};
    std::uint64_t m_maxLines{std::numeric_limits<std::uint64_t>::max()};
};

} // namespace scope::investigation
