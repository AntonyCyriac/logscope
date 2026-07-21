/**
 * @file text_matcher.hpp
 * @brief Substring matching helpers for search queries.
 */

#pragma once

#include <string_view>

#include "search_query.hpp"

namespace scope::search
{

/**
 * @brief Determines whether haystack contains needle using the requested case mode.
 */
[[nodiscard]] bool textContains(std::string_view haystack, std::string_view needle,
                                CaseSensitivity caseSensitivity) noexcept;

} // namespace scope::search
