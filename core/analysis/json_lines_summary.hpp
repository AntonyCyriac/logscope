/**
 * @file json_lines_summary.hpp
 * @brief Aggregated statistics for JSON Lines analysis.
 */

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace scope::analysis
{

/**
 * @brief Summary of JSON Lines parsing results.
 */
class JsonLinesSummary
{
  public:
    /**
     * @brief Records a successfully parsed JSON object line.
     *
     * @param topLevelKeys Top-level keys observed in the parsed object.
     */
    void recordValidLine(const std::vector<std::string>& topLevelKeys);

    /**
     * @brief Records a line that could not be parsed as JSON.
     */
    void recordParseFailure() noexcept;

    /**
     * @brief Returns the number of valid JSON object lines.
     */
    [[nodiscard]] std::uint64_t validLines() const noexcept;

    /**
     * @brief Returns the number of lines that failed JSON parsing.
     */
    [[nodiscard]] std::uint64_t parseFailures() const noexcept;

    /**
     * @brief Returns the most frequent top-level keys.
     *
     * @param limit Maximum number of keys to return.
     */
    [[nodiscard]] std::vector<std::pair<std::string, std::uint64_t>>
    topLevelKeys(std::size_t limit = 10U) const;

  private:
    std::uint64_t m_validLines{0U};
    std::uint64_t m_parseFailures{0U};
    std::unordered_map<std::string, std::uint64_t> m_keyCounts;
};

} // namespace scope::analysis
