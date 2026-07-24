/**
 * @file query_node.hpp
 * @brief Filter query abstract syntax tree.
 */

#pragma once

#include <memory>
#include <string>

#include "query_value.hpp"

namespace scope::query
{

enum class ComparisonOperator
{
    Equal,
    NotEqual,
    Greater,
    GreaterEqual,
    Less,
    LessEqual
};

enum class FunctionKind
{
    Contains,
    HasKey
};

/**
 * @brief Structured filter query supporting field predicates and boolean composition.
 */
class QueryNode
{
  public:
    enum class Kind
    {
        MatchAll,
        Comparison,
        FunctionCall,
        And,
        Or,
        Not
    };

    QueryNode();

    QueryNode(QueryNode&&) noexcept;
    QueryNode& operator=(QueryNode&&) noexcept;

    QueryNode(const QueryNode& other);
    QueryNode& operator=(const QueryNode& other);

    ~QueryNode();

    [[nodiscard]] static QueryNode matchAll() noexcept;

    [[nodiscard]] static QueryNode comparison(std::string field, ComparisonOperator op, QueryValue value);

    [[nodiscard]] static QueryNode functionCall(FunctionKind function, std::string field, std::string argument);

    [[nodiscard]] static QueryNode andNode(QueryNode left, QueryNode right);

    [[nodiscard]] static QueryNode orNode(QueryNode left, QueryNode right);

    [[nodiscard]] static QueryNode notNode(QueryNode operand);

    [[nodiscard]] Kind kind() const noexcept;

    [[nodiscard]] const std::string& field() const noexcept;

    [[nodiscard]] ComparisonOperator comparisonOperator() const noexcept;

    [[nodiscard]] const QueryValue& value() const noexcept;

    [[nodiscard]] FunctionKind functionKind() const noexcept;

    [[nodiscard]] const std::string& argument() const noexcept;

    [[nodiscard]] const QueryNode* left() const noexcept;

    [[nodiscard]] const QueryNode* right() const noexcept;

    [[nodiscard]] const QueryNode* operand() const noexcept;

    [[nodiscard]] bool isActive() const noexcept;

    [[nodiscard]] std::string toString() const;

  private:
    struct Impl;

    explicit QueryNode(std::unique_ptr<Impl> impl);

    std::unique_ptr<Impl> m_impl;
};

} // namespace scope::query
