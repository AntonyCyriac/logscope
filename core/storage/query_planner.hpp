/**
 * @file query_planner.hpp
 * @brief SQL pushdown planner for M10 filter queries.
 */

#pragma once

#include <optional>
#include <string>

#include "query_node.hpp"

namespace scope::storage
{

struct QueryPlan
{
    std::string sqlWhere;
};

/**
 * @brief Attempts to translate a filter query AST into a SQL WHERE clause.
 *
 * @return std::nullopt when the query cannot be fully pushed down safely.
 */
[[nodiscard]] std::optional<QueryPlan> planQueryPushdown(const query::QueryNode& root) noexcept;

} // namespace scope::storage
