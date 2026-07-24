/**
 * @file query_evaluator.cpp
 */

#include "query_evaluator.hpp"

#include <algorithm>

#include "foundation/string.hpp"
#include "foundation/timestamp.hpp"

namespace scope::query
{

namespace
{

std::string normalizeFieldName(const std::string& field)
{
    return foundation::toLower(field);
}

bool containsCaseInsensitive(const std::string_view haystack, const std::string_view needle)
{
    if (needle.empty())
    {
        return true;
    }

    const std::string loweredHaystack = foundation::toLower(haystack);
    const std::string loweredNeedle = foundation::toLower(needle);

    return loweredHaystack.find(loweredNeedle) != std::string::npos;
}

std::optional<foundation::Timestamp> literalTimestamp(const QueryValue& value)
{
    if (value.kind() == QueryValue::Kind::String)
    {
        const auto parsed = foundation::Timestamp::parse(value.stringValue());

        if (parsed.hasValue())
        {
            return *parsed;
        }
    }

    return std::nullopt;
}

} // namespace

QueryEvaluator::QueryEvaluator(QueryNode root) noexcept : m_root(std::move(root)) {}

bool QueryEvaluator::isActive() const noexcept
{
    return m_root.isActive();
}

bool QueryEvaluator::matches(const analysis::IndexedLine& line) const noexcept
{
    if (!isActive())
    {
        return true;
    }

    return matchesNode(m_root, line);
}

bool QueryEvaluator::matchesNode(const QueryNode& node, const analysis::IndexedLine& line) const noexcept
{
    switch (node.kind())
    {
    case QueryNode::Kind::MatchAll:
        return true;
    case QueryNode::Kind::Comparison:
        return evaluateComparison(node, line);
    case QueryNode::Kind::FunctionCall:
        return evaluateFunction(node, line);
    case QueryNode::Kind::And:
        return matchesNode(*node.left(), line) && matchesNode(*node.right(), line);
    case QueryNode::Kind::Or:
        return matchesNode(*node.left(), line) || matchesNode(*node.right(), line);
    case QueryNode::Kind::Not:
        return !matchesNode(*node.operand(), line);
    }

    return false;
}

bool QueryEvaluator::evaluateComparison(const QueryNode& node, const analysis::IndexedLine& line) const noexcept
{
    const std::string field = normalizeFieldName(node.field());
    const QueryValue& literal = node.value();

    if (field == "level")
    {
        if (literal.kind() != QueryValue::Kind::Level)
        {
            return false;
        }

        const analysis::DetectedLogLevel lineLevel = line.level;
        const analysis::DetectedLogLevel expected = literal.levelValue();

        switch (node.comparisonOperator())
        {
        case ComparisonOperator::Equal:
            return lineLevel == expected;
        case ComparisonOperator::NotEqual:
            return lineLevel != expected;
        default:
            return false;
        }
    }

    if (field == "line")
    {
        if (literal.kind() != QueryValue::Kind::Number)
        {
            return false;
        }

        const std::uint64_t lineNumber = line.lineNumber;
        const std::uint64_t expected = literal.numberValue();

        switch (node.comparisonOperator())
        {
        case ComparisonOperator::Equal:
            return lineNumber == expected;
        case ComparisonOperator::NotEqual:
            return lineNumber != expected;
        case ComparisonOperator::Greater:
            return lineNumber > expected;
        case ComparisonOperator::GreaterEqual:
            return lineNumber >= expected;
        case ComparisonOperator::Less:
            return lineNumber < expected;
        case ComparisonOperator::LessEqual:
            return lineNumber <= expected;
        }
    }

    if (field == "time" || field == "timestamp")
    {
        if (!line.timestamp.has_value())
        {
            return false;
        }

        const foundation::Timestamp actual = *line.timestamp;

        if (literal.kind() == QueryValue::Kind::Number)
        {
            const foundation::Timestamp expected =
                foundation::Timestamp::fromUnixSeconds(static_cast<std::int64_t>(literal.numberValue()));

            switch (node.comparisonOperator())
            {
            case ComparisonOperator::Equal:
                return actual.unixSeconds() == expected.unixSeconds();
            case ComparisonOperator::NotEqual:
                return actual.unixSeconds() != expected.unixSeconds();
            case ComparisonOperator::Greater:
                return actual.unixSeconds() > expected.unixSeconds();
            case ComparisonOperator::GreaterEqual:
                return actual.unixSeconds() >= expected.unixSeconds();
            case ComparisonOperator::Less:
                return actual.unixSeconds() < expected.unixSeconds();
            case ComparisonOperator::LessEqual:
                return actual.unixSeconds() <= expected.unixSeconds();
            }
        }

        if (literal.kind() == QueryValue::Kind::String)
        {
            const auto expected = literalTimestamp(literal);

            if (!expected.has_value())
            {
                return false;
            }

            switch (node.comparisonOperator())
            {
            case ComparisonOperator::Equal:
                return actual.unixSeconds() == expected->unixSeconds();
            case ComparisonOperator::NotEqual:
                return actual.unixSeconds() != expected->unixSeconds();
            case ComparisonOperator::Greater:
                return actual.unixSeconds() > expected->unixSeconds();
            case ComparisonOperator::GreaterEqual:
                return actual.unixSeconds() >= expected->unixSeconds();
            case ComparisonOperator::Less:
                return actual.unixSeconds() < expected->unixSeconds();
            case ComparisonOperator::LessEqual:
                return actual.unixSeconds() <= expected->unixSeconds();
            }
        }

        return false;
    }

    if (field == "message")
    {
        if (literal.kind() != QueryValue::Kind::String)
        {
            return false;
        }

        switch (node.comparisonOperator())
        {
        case ComparisonOperator::Equal:
            return foundation::toLower(line.messageExcerpt) == foundation::toLower(literal.stringValue());
        case ComparisonOperator::NotEqual:
            return foundation::toLower(line.messageExcerpt) != foundation::toLower(literal.stringValue());
        default:
            return false;
        }
    }

    if (field == "content")
    {
        if (literal.kind() != QueryValue::Kind::String)
        {
            return false;
        }

        switch (node.comparisonOperator())
        {
        case ComparisonOperator::Equal:
            return foundation::toLower(line.contentExcerpt) == foundation::toLower(literal.stringValue());
        case ComparisonOperator::NotEqual:
            return foundation::toLower(line.contentExcerpt) != foundation::toLower(literal.stringValue());
        default:
            return false;
        }
    }

    if (field == "correlationid")
    {
        if (literal.kind() != QueryValue::Kind::String)
        {
            return false;
        }

        switch (node.comparisonOperator())
        {
        case ComparisonOperator::Equal:
            return line.correlationId == literal.stringValue();
        case ComparisonOperator::NotEqual:
            return line.correlationId != literal.stringValue();
        default:
            return containsCaseInsensitive(line.correlationId, literal.stringValue());
        }
    }

    return false;
}

bool QueryEvaluator::evaluateFunction(const QueryNode& node, const analysis::IndexedLine& line) const noexcept
{
    if (node.functionKind() == FunctionKind::HasKey)
    {
        return std::find(line.topLevelKeys.begin(), line.topLevelKeys.end(), node.argument()) !=
               line.topLevelKeys.end();
    }

    const std::string field = normalizeFieldName(node.field());

    if (field == "message")
    {
        return containsCaseInsensitive(line.messageExcerpt, node.argument());
    }

    if (field == "content")
    {
        return containsCaseInsensitive(line.contentExcerpt, node.argument());
    }

    if (field == "correlationid")
    {
        return containsCaseInsensitive(line.correlationId, node.argument());
    }

    return false;
}

} // namespace scope::query
