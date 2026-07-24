/**
 * @file query_planner.cpp
 */

#include "query_planner.hpp"

#include <sstream>

#include "foundation/string.hpp"
#include "foundation/timestamp.hpp"
#include "log_line_classifier.hpp"

namespace scope::storage
{

namespace
{

[[nodiscard]] std::string escapeSqlLiteral(const std::string& value)
{
    std::string escaped;
    escaped.reserve(value.size());

    for (const char character : value)
    {
        if (character == '\'')
        {
            escaped += "''";
        }
        else
        {
            escaped += character;
        }
    }

    return escaped;
}

[[nodiscard]] int levelToInt(const analysis::DetectedLogLevel level) noexcept
{
    switch (level)
    {
    case analysis::DetectedLogLevel::Blank:
        return 0;
    case analysis::DetectedLogLevel::Info:
        return 1;
    case analysis::DetectedLogLevel::Warn:
        return 2;
    case analysis::DetectedLogLevel::Error:
        return 3;
    case analysis::DetectedLogLevel::Other:
        return 4;
    }

    return 4;
}

[[nodiscard]] std::optional<std::string> comparisonSql(const query::QueryNode& node)
{
    const std::string field = foundation::toLower(node.field());
    const query::QueryValue& literal = node.value();

    auto wrap = [](const std::string& expression) {
        return std::optional<std::string>(expression);
    };

    if (field == "level")
    {
        if (literal.kind() != query::QueryValue::Kind::Level)
        {
            return std::nullopt;
        }

        const std::string value = std::to_string(levelToInt(literal.levelValue()));

        switch (node.comparisonOperator())
        {
        case query::ComparisonOperator::Equal:
            return wrap("level = " + value);
        case query::ComparisonOperator::NotEqual:
            return wrap("level <> " + value);
        default:
            return std::nullopt;
        }
    }

    if (field == "line")
    {
        if (literal.kind() != query::QueryValue::Kind::Number)
        {
            return std::nullopt;
        }

        const std::string value = std::to_string(literal.numberValue());
        const char* op = "=";

        switch (node.comparisonOperator())
        {
        case query::ComparisonOperator::Equal:
            op = "=";
            break;
        case query::ComparisonOperator::NotEqual:
            op = "<>";
            break;
        case query::ComparisonOperator::Greater:
            op = ">";
            break;
        case query::ComparisonOperator::GreaterEqual:
            op = ">=";
            break;
        case query::ComparisonOperator::Less:
            op = "<";
            break;
        case query::ComparisonOperator::LessEqual:
            op = "<=";
            break;
        }

        return wrap("line_number " + std::string(op) + " " + value);
    }

    if (field == "time" || field == "timestamp")
    {
        std::optional<foundation::Timestamp> timestamp;

        if (literal.kind() == query::QueryValue::Kind::String)
        {
            const auto parsed = foundation::Timestamp::parse(literal.stringValue());

            if (parsed.hasValue())
            {
                timestamp = *parsed;
            }
        }

        if (!timestamp.has_value())
        {
            return std::nullopt;
        }

        const std::string value = std::to_string(timestamp->unixSeconds());
        const char* op = "=";

        switch (node.comparisonOperator())
        {
        case query::ComparisonOperator::Equal:
            op = "=";
            break;
        case query::ComparisonOperator::NotEqual:
            op = "<>";
            break;
        case query::ComparisonOperator::Greater:
            op = ">";
            break;
        case query::ComparisonOperator::GreaterEqual:
            op = ">=";
            break;
        case query::ComparisonOperator::Less:
            op = "<";
            break;
        case query::ComparisonOperator::LessEqual:
            op = "<=";
            break;
        }

        return wrap("timestamp_unix " + std::string(op) + " " + value);
    }

    if (field == "correlationid")
    {
        if (literal.kind() != query::QueryValue::Kind::String)
        {
            return std::nullopt;
        }

        const std::string escaped = escapeSqlLiteral(literal.stringValue());

        switch (node.comparisonOperator())
        {
        case query::ComparisonOperator::Equal:
            return wrap("correlation_id = '" + escaped + "'");
        case query::ComparisonOperator::NotEqual:
            return wrap("correlation_id <> '" + escaped + "'");
        default:
            return std::nullopt;
        }
    }

    if (field == "message" || field == "content")
    {
        if (literal.kind() != query::QueryValue::Kind::String)
        {
            return std::nullopt;
        }

        if (node.comparisonOperator() != query::ComparisonOperator::Equal)
        {
            return std::nullopt;
        }

        const std::string column = field == "message" ? "message" : "content";
        const std::string escaped = escapeSqlLiteral(literal.stringValue());

        return wrap(column + " = '" + escaped + "'");
    }

    return std::nullopt;
}

[[nodiscard]] std::optional<std::string> functionSql(const query::QueryNode& node)
{
    if (node.functionKind() == query::FunctionKind::Contains)
    {
        const std::string field = foundation::toLower(node.field());

        if (field != "message" && field != "content")
        {
            return std::nullopt;
        }

        const std::string column = field == "message" ? "message" : "content";
        const std::string escaped = escapeSqlLiteral(node.argument());

        return std::optional<std::string>("LOWER(" + column + ") LIKE '%" + foundation::toLower(escaped) + "%'");
    }

    if (node.functionKind() == query::FunctionKind::HasKey)
    {
        const std::string escaped = escapeSqlLiteral(node.argument());

        return std::optional<std::string>(
            "(top_level_keys_json = '" + escaped + "' OR top_level_keys_json LIKE '" + escaped +
            ";%' OR top_level_keys_json LIKE '%;" + escaped + "' OR top_level_keys_json LIKE '%;" + escaped + ";%')");
    }

    return std::nullopt;
}

[[nodiscard]] std::optional<std::string> planNode(const query::QueryNode& node)
{
    switch (node.kind())
    {
    case query::QueryNode::Kind::MatchAll:
        return std::optional<std::string>("1 = 1");
    case query::QueryNode::Kind::Comparison:
        return comparisonSql(node);
    case query::QueryNode::Kind::FunctionCall:
        return functionSql(node);
    case query::QueryNode::Kind::And:
    {
        const auto left = planNode(*node.left());
        const auto right = planNode(*node.right());

        if (!left || !right)
        {
            return std::nullopt;
        }

        return std::optional<std::string>("(" + *left + " AND " + *right + ")");
    }
    case query::QueryNode::Kind::Or:
    {
        const auto left = planNode(*node.left());
        const auto right = planNode(*node.right());

        if (!left || !right)
        {
            return std::nullopt;
        }

        return std::optional<std::string>("(" + *left + " OR " + *right + ")");
    }
    case query::QueryNode::Kind::Not:
    {
        const auto operand = planNode(*node.operand());

        if (!operand)
        {
            return std::nullopt;
        }

        return std::optional<std::string>("NOT (" + *operand + ")");
    }
    }

    return std::nullopt;
}

} // namespace

std::optional<QueryPlan> planQueryPushdown(const query::QueryNode& root) noexcept
{
    if (!root.isActive())
    {
        return QueryPlan{"1 = 1"};
    }

    const auto sql = planNode(root);

    if (!sql)
    {
        return std::nullopt;
    }

    return QueryPlan{*sql};
}

} // namespace scope::storage
