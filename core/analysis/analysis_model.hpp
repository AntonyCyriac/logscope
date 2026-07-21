/**
 * @file analysis_model.hpp
 * @brief Represents the canonical analysis result (DO-002).
 */

#pragma once

#include <cstdint>
#include <optional>

#include "field_summary.hpp"
#include "foundation/path.hpp"
#include "json_lines_summary.hpp"
#include "log_format.hpp"
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
     * @param format Detected or overridden log format.
     */
    AnalysisModel(foundation::Path sourcePath, std::uint64_t totalLines,
                  LogLevelCounts levelCounts = {}, LogFormat format = LogFormat::PlainText,
                  std::optional<JsonLinesSummary> jsonLinesSummary = std::nullopt,
                  std::optional<FieldSummary> fieldSummary = std::nullopt) noexcept;

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

    /**
     * @brief Returns the detected or overridden log format.
     */
    [[nodiscard]] LogFormat format() const noexcept;

    /**
     * @brief Returns JSON Lines parsing statistics when format is JsonLines.
     */
    [[nodiscard]] const std::optional<JsonLinesSummary>& jsonLinesSummary() const noexcept;

    /**
     * @brief Returns extracted timestamp and message field statistics.
     */
    [[nodiscard]] const std::optional<FieldSummary>& fieldSummary() const noexcept;

  private:
    foundation::Path m_sourcePath;
    std::uint64_t m_totalLines;
    LogLevelCounts m_levelCounts;
    LogFormat m_format;
    std::optional<JsonLinesSummary> m_jsonLinesSummary;
    std::optional<FieldSummary> m_fieldSummary;
};

} // namespace scope::analysis
