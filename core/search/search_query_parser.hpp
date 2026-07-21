/**
 * @file search_query_parser.hpp
 * @brief Parses boolean search query expressions.
 */

#pragma once

#include <string_view>

#include "foundation/result.hpp"
#include "search_query.hpp"

namespace scope::search
{

/**
 * @brief Parses a boolean search expression into a SearchQuery tree.
 */
[[nodiscard]] foundation::Result<SearchQuery> parseSearchQuery(std::string_view expression) noexcept;

/**
 * @brief Parses a regex pattern into a SearchQuery term.
 */
[[nodiscard]] foundation::Result<SearchQuery> parseRegexQuery(std::string_view pattern,
                                                             CaseSensitivity caseSensitivity) noexcept;

} // namespace scope::search
