/**
 * @file analysis_model.hpp
 * @brief Represents the canonical analysis result (DO-002).
 */

#pragma once

#include <cstdint>

#include "foundation/path.hpp"
#include "log_level_counts.hpp"

namespace scope::analysis
{

/**
 * @brief Canonical representation of analyzed log information.
 */
class AnalysisModel
{
  public:
    /**
     * @brief Constructs an analysis model.
     *
     * @param sourcePath Path of the analyzed source.
     * @param totalLines Number of log lines analyzed.
     * @param levelCounts Per-level line statistics.
     */
    AnalysisModel(foundation::Path sourcePath, std::uint64_t totalLines,
                  LogLevelCounts levelCounts = {}) noexcept;

    /**
     * @brief Returns the path of the analyzed source.
     */
    [[nodiscard]] const foundation::Path& sourcePath() const noexcept;

    /**
     * @brief Returns the total number of log lines analyzed.
     */
    [[nodiscard]] std::uint64_t totalLines() const noexcept;

    /**
     * @brief Returns per-level line statistics.
     */
    [[nodiscard]] const LogLevelCounts& levelCounts() const noexcept;

  private:
    foundation::Path m_sourcePath;
    std::uint64_t m_totalLines;
    LogLevelCounts m_levelCounts;
};

} // namespace scope::analysis
