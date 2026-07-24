/**
 * @file query_evaluator.hpp
 * @brief Evaluates filter query AST nodes against indexed log lines.
 */

#pragma once

#include "line_index.hpp"
#include "query_node.hpp"

namespace scope::query
{

/**
 * @brief Evaluates a filter query against indexed line metadata.
 */
class QueryEvaluator
{
  public:
    explicit QueryEvaluator(QueryNode root) noexcept;

    [[nodiscard]] bool isActive() const noexcept;

    [[nodiscard]] bool matches(const analysis::IndexedLine& line) const noexcept;

  private:
    [[nodiscard]] bool matchesNode(const QueryNode& node, const analysis::IndexedLine& line) const noexcept;

    [[nodiscard]] bool evaluateComparison(const QueryNode& node, const analysis::IndexedLine& line) const noexcept;

    [[nodiscard]] bool evaluateFunction(const QueryNode& node, const analysis::IndexedLine& line) const noexcept;

    QueryNode m_root;
};

} // namespace scope::query
