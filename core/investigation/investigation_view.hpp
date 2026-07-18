/**
 * @file investigation_view.hpp
 * @brief Read-only inspection of an analysis model.
 */

#pragma once

#include <cstdint>
#include <string>

#include "analysis_model.hpp"
#include "foundation/path.hpp"

namespace scope::investigation
{

/**
 * @brief Detailed, read-only view of analyzed log information.
 */
class InvestigationView
{
  public:
    /**
     * @brief Constructs an investigation view from an analysis model.
     */
    explicit InvestigationView(const analysis::AnalysisModel& model) noexcept;

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
    [[nodiscard]] const analysis::LogLevelCounts& levelCounts() const noexcept;

    /**
     * @brief Determines whether the analysis contains no log lines.
     */
    [[nodiscard]] bool isEmpty() const noexcept;

    /**
     * @brief Returns a human-readable summary for inspection.
     */
    [[nodiscard]] std::string summary() const;

  private:
    foundation::Path m_sourcePath;
    std::uint64_t m_totalLines;
    analysis::LogLevelCounts m_levelCounts;
};

} // namespace scope::investigation
