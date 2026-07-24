/**
 * @file query_parser.hpp
 * @brief Filter query DSL parser.
 */

#pragma once

#include <string_view>

#include "foundation/result.hpp"
#include "query_node.hpp"

namespace scope::query
{

/**
 * @brief Parses a field-aware filter query expression.
 */
[[nodiscard]] foundation::Result<QueryNode> parseFilterQuery(std::string_view input);

} // namespace scope::query
