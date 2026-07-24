/**
 * @file query_node.cpp
 */

#include "query_node.hpp"

#include <sstream>
#include <utility>

namespace scope::query
{

namespace
{

std::string comparisonOperatorName(const ComparisonOperator op)
{
    switch (op)
    {
    case ComparisonOperator::Equal:
        return "==";
    case ComparisonOperator::NotEqual:
        return "!=";
    case ComparisonOperator::Greater:
        return ">";
    case ComparisonOperator::GreaterEqual:
        return ">=";
    case ComparisonOperator::Less:
        return "<";
    case ComparisonOperator::LessEqual:
        return "<=";
    }

    return "==";
}

std::string levelKeyword(const analysis::DetectedLogLevel level)
{
    switch (level)
    {
    case analysis::DetectedLogLevel::Error:
        return "ERROR";
    case analysis::DetectedLogLevel::Warn:
        return "WARNING";
    case analysis::DetectedLogLevel::Info:
        return "INFO";
    case analysis::DetectedLogLevel::Blank:
        return "BLANK";
    case analysis::DetectedLogLevel::Other:
        return "OTHER";
    }

    return "OTHER";
}

std::string valueToString(const QueryValue& value)
{
    switch (value.kind())
    {
    case QueryValue::Kind::String:
        return '"' + value.stringValue() + '"';
    case QueryValue::Kind::Number:
        return std::to_string(value.numberValue());
    case QueryValue::Kind::Level:
        return levelKeyword(value.levelValue());
    }

    return "";
}

} // namespace

struct QueryNode::Impl
{
    Kind kind{Kind::MatchAll};
    std::string field;
    ComparisonOperator comparisonOperator{ComparisonOperator::Equal};
    QueryValue value = QueryValue::stringValue("");
    FunctionKind functionKind{FunctionKind::Contains};
    std::string argument;
    std::unique_ptr<QueryNode> left;
    std::unique_ptr<QueryNode> right;
    std::unique_ptr<QueryNode> operand;
};

QueryNode::QueryNode() : m_impl(std::make_unique<Impl>()) {}

QueryNode::QueryNode(std::unique_ptr<Impl> impl) : m_impl(std::move(impl)) {}

QueryNode::QueryNode(QueryNode&&) noexcept = default;

QueryNode& QueryNode::operator=(QueryNode&&) noexcept = default;

QueryNode::QueryNode(const QueryNode& other) : m_impl(std::make_unique<Impl>())
{
    m_impl->kind = other.m_impl->kind;
    m_impl->field = other.m_impl->field;
    m_impl->comparisonOperator = other.m_impl->comparisonOperator;
    m_impl->value = other.m_impl->value;
    m_impl->functionKind = other.m_impl->functionKind;
    m_impl->argument = other.m_impl->argument;

    if (other.m_impl->left)
    {
        m_impl->left = std::make_unique<QueryNode>(*other.m_impl->left);
    }

    if (other.m_impl->right)
    {
        m_impl->right = std::make_unique<QueryNode>(*other.m_impl->right);
    }

    if (other.m_impl->operand)
    {
        m_impl->operand = std::make_unique<QueryNode>(*other.m_impl->operand);
    }
}

QueryNode& QueryNode::operator=(const QueryNode& other)
{
    if (this != &other)
    {
        *this = QueryNode(other);
    }

    return *this;
}

QueryNode::~QueryNode() = default;

QueryNode QueryNode::matchAll() noexcept
{
    return QueryNode();
}

QueryNode QueryNode::comparison(std::string field, const ComparisonOperator op, QueryValue value)
{
    auto impl = std::make_unique<Impl>();
    impl->kind = Kind::Comparison;
    impl->field = std::move(field);
    impl->comparisonOperator = op;
    impl->value = std::move(value);

    return QueryNode(std::move(impl));
}

QueryNode QueryNode::functionCall(const FunctionKind function, std::string field, std::string argument)
{
    auto impl = std::make_unique<Impl>();
    impl->kind = Kind::FunctionCall;
    impl->functionKind = function;
    impl->field = std::move(field);
    impl->argument = std::move(argument);

    return QueryNode(std::move(impl));
}

QueryNode QueryNode::andNode(QueryNode left, QueryNode right)
{
    auto impl = std::make_unique<Impl>();
    impl->kind = Kind::And;
    impl->left = std::make_unique<QueryNode>(std::move(left));
    impl->right = std::make_unique<QueryNode>(std::move(right));

    return QueryNode(std::move(impl));
}

QueryNode QueryNode::orNode(QueryNode left, QueryNode right)
{
    auto impl = std::make_unique<Impl>();
    impl->kind = Kind::Or;
    impl->left = std::make_unique<QueryNode>(std::move(left));
    impl->right = std::make_unique<QueryNode>(std::move(right));

    return QueryNode(std::move(impl));
}

QueryNode QueryNode::notNode(QueryNode operand)
{
    auto impl = std::make_unique<Impl>();
    impl->kind = Kind::Not;
    impl->operand = std::make_unique<QueryNode>(std::move(operand));

    return QueryNode(std::move(impl));
}

QueryNode::Kind QueryNode::kind() const noexcept
{
    return m_impl->kind;
}

const std::string& QueryNode::field() const noexcept
{
    return m_impl->field;
}

ComparisonOperator QueryNode::comparisonOperator() const noexcept
{
    return m_impl->comparisonOperator;
}

const QueryValue& QueryNode::value() const noexcept
{
    return m_impl->value;
}

FunctionKind QueryNode::functionKind() const noexcept
{
    return m_impl->functionKind;
}

const std::string& QueryNode::argument() const noexcept
{
    return m_impl->argument;
}

const QueryNode* QueryNode::left() const noexcept
{
    return m_impl->left.get();
}

const QueryNode* QueryNode::right() const noexcept
{
    return m_impl->right.get();
}

const QueryNode* QueryNode::operand() const noexcept
{
    return m_impl->operand.get();
}

bool QueryNode::isActive() const noexcept
{
    return m_impl->kind != Kind::MatchAll;
}

std::string QueryNode::toString() const
{
    switch (m_impl->kind)
    {
    case Kind::MatchAll:
        return "*";
    case Kind::Comparison:
        return m_impl->field + ' ' + comparisonOperatorName(m_impl->comparisonOperator) + ' ' +
               valueToString(m_impl->value);
    case Kind::FunctionCall:
        if (m_impl->functionKind == FunctionKind::HasKey)
        {
            return "hasKey(" + m_impl->argument + ')';
        }

        return "contains(" + m_impl->field + ", " + valueToString(QueryValue::stringValue(m_impl->argument)) + ')';
    case Kind::And:
        return '(' + m_impl->left->toString() + " AND " + m_impl->right->toString() + ')';
    case Kind::Or:
        return '(' + m_impl->left->toString() + " OR " + m_impl->right->toString() + ')';
    case Kind::Not:
        return "(NOT " + m_impl->operand->toString() + ')';
    }

    return "";
}

} // namespace scope::query
