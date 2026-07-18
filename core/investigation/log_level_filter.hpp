/**
 * @file log_level_filter.hpp
 * @brief Filters analysis models by log level statistics.
 */

#pragma once

#include <cstdint>

#include "analysis_model.hpp"

namespace scope::investigation
{

/**
 * @brief Progressive filter for minimum log level counts.
 */
class LogLevelFilter
{
  public:
    /**
     * @brief Creates a filter that matches any log level counts.
     */
    [[nodiscard]] static LogLevelFilter any() noexcept;

    /**
     * @brief Returns a copy requiring at least the given number of error lines.
     */
    [[nodiscard]] LogLevelFilter withMinimumErrors(std::uint64_t minErrors) const noexcept;

    /**
     * @brief Returns a copy requiring at least the given number of warning lines.
     */
    [[nodiscard]] LogLevelFilter withMinimumWarnings(std::uint64_t minWarnings) const noexcept;

    /**
     * @brief Determines whether the model satisfies this filter.
     */
    [[nodiscard]] bool matches(const analysis::AnalysisModel& model) const noexcept;

  private:
    std::uint64_t m_minErrors{0U};
    std::uint64_t m_minWarnings{0U};
};

} // namespace scope::investigation
