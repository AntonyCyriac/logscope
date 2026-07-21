/**
 * @file line_index.hpp
 * @brief Bounded per-line index for content-aware investigation.
 */

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "foundation/timestamp.hpp"
#include "log_line_classifier.hpp"

namespace scope::analysis
{

/// Maximum lines stored in the investigation index.
constexpr std::size_t maxIndexedLines = 10000U;

/// Maximum characters stored per line for content search.
constexpr std::size_t maxLineContentExcerptLength = 256U;

/**
 * @brief Indexed metadata for a single analyzed log line.
 */
struct IndexedLine
{
    std::uint64_t lineNumber{0U};
    DetectedLogLevel level{DetectedLogLevel::Other};
    std::optional<foundation::Timestamp> timestamp;
    std::string messageExcerpt;
    std::string correlationId;
    std::string contentExcerpt;
    std::vector<std::string> topLevelKeys;
};

/**
 * @brief Bounded store of per-line metadata built during analysis.
 */
class LineIndex
{
  public:
    /**
     * @brief Attempts to add a line to the index.
     *
     * @return true when stored, false when the index is full.
     */
    [[nodiscard]] bool tryAddLine(IndexedLine line) noexcept;

    [[nodiscard]] const std::vector<IndexedLine>& lines() const noexcept;

    [[nodiscard]] std::uint64_t indexedLineCount() const noexcept;

    [[nodiscard]] std::uint64_t truncatedLineCount() const noexcept;

  private:
    std::vector<IndexedLine> m_lines;
    std::uint64_t m_truncatedLines{0U};
};

/**
 * @brief Truncates text to a maximum length.
 */
[[nodiscard]] std::string truncateExcerpt(std::string_view value, std::size_t maxLength) noexcept;

} // namespace scope::analysis
