/**
 * @file search_engine.hpp
 * @brief Executes search queries against indexed log lines.
 */

#pragma once

#include <string_view>
#include <vector>

#include "line_index.hpp"
#include "search_query.hpp"

namespace scope::search
{

/// Maximum allowed regex pattern length.
constexpr std::size_t maxRegexPatternLength = 512U;

/**
 * @brief Matches and searches indexed log lines.
 */
class SearchEngine
{
  public:
    /**
     * @brief Determines whether a single indexed line matches the query.
     */
    [[nodiscard]] bool matches(const analysis::IndexedLine& line, const SearchQuery& query) const noexcept;

    /**
     * @brief Returns indexed lines that match the query.
     */
    [[nodiscard]] std::vector<analysis::IndexedLine> search(const analysis::LineIndex& index,
                                                            const SearchQuery& query) const;

    /**
     * @brief Returns indexed lines that match the query.
     */
    [[nodiscard]] std::vector<analysis::IndexedLine> search(const std::vector<analysis::IndexedLine>& lines,
                                                            const SearchQuery& query) const;
};

} // namespace scope::search
