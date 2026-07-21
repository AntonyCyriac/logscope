/**
 * @file field_summary.hpp
 * @brief Aggregated timestamp and message field statistics.
 */

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "foundation/timestamp.hpp"

namespace scope::analysis
{

/// Maximum characters stored per recurring message pattern.
constexpr std::size_t maxMessageExcerptLength = 120U;

/**
 * @brief Summary of extracted timestamp and message fields.
 */
class FieldSummary
{
  public:
    /**
     * @brief Records a parsed timestamp from a log line.
     */
    void recordTimestamp(const foundation::Timestamp& timestamp) noexcept;

    /**
     * @brief Records a message excerpt for pattern counting.
     *
     * @param message Message text (truncated internally).
     */
    void recordMessage(std::string_view message);

    /**
     * @brief Returns the earliest observed timestamp.
     */
    [[nodiscard]] const std::optional<foundation::Timestamp>& earliestTimestamp() const noexcept;

    /**
     * @brief Returns the latest observed timestamp.
     */
    [[nodiscard]] const std::optional<foundation::Timestamp>& latestTimestamp() const noexcept;

    /**
     * @brief Returns how many lines contributed a timestamp.
     */
    [[nodiscard]] std::uint64_t linesWithTimestamp() const noexcept;

    /**
     * @brief Returns how many lines contributed a message excerpt.
     */
    [[nodiscard]] std::uint64_t linesWithMessage() const noexcept;

    /**
     * @brief Returns the most frequent message excerpts.
     *
     * @param limit Maximum number of patterns to return.
     */
    [[nodiscard]] std::vector<std::pair<std::string, std::uint64_t>>
    topMessages(std::size_t limit = 10U) const;

  private:
    std::optional<foundation::Timestamp> m_earliestTimestamp;
    std::optional<foundation::Timestamp> m_latestTimestamp;
    std::uint64_t m_linesWithTimestamp{0U};
    std::uint64_t m_linesWithMessage{0U};
    std::unordered_map<std::string, std::uint64_t> m_messageCounts;
};

/**
 * @brief Truncates a message excerpt for storage and counting.
 */
[[nodiscard]] std::string normalizeMessageExcerpt(std::string_view message);

} // namespace scope::analysis
