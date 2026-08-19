/**
 * @file query_evaluator.cpp
 */

#include "query_evaluator.hpp"

#include <algorithm>
#include <charconv>
#include <cmath>

#include "foundation/error.hpp"
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

[[nodiscard]] bool isReservedComparisonField(const std::string_view field) noexcept
{
    const std::string lower = foundation::toLower(field);

    return lower == "level" || lower == "line" || lower == "time" || lower == "timestamp" ||
           lower == "correlationid" || lower == "message" || lower == "content";
}

[[nodiscard]] bool isOrderedComparisonOperator(const ComparisonOperator comparisonOperator) noexcept
{
    switch (comparisonOperator)
    {
    case ComparisonOperator::Greater:
    case ComparisonOperator::GreaterEqual:
    case ComparisonOperator::Less:
    case ComparisonOperator::LessEqual:
        return true;
    case ComparisonOperator::Equal:
    case ComparisonOperator::NotEqual:
        return false;
    }

    return false;
}

[[nodiscard]] bool tryParseJsonFieldNumber(const std::string_view text, double& value) noexcept
{
    if (text.empty())
    {
        return false;
    }

    const char* begin = text.data();
    const char* end = begin + text.size();
    const auto parsed = std::from_chars(begin, end, value);

    if (parsed.ec != std::errc{} || parsed.ptr != end)
    {
        return false;
    }

    return std::isfinite(value);
}

[[nodiscard]] bool compareNumeric(const double actual, const double expected,
                                  const ComparisonOperator comparisonOperator) noexcept
{
    switch (comparisonOperator)
    {
    case ComparisonOperator::Equal:
        return actual == expected;
    case ComparisonOperator::NotEqual:
        return actual != expected;
    case ComparisonOperator::Greater:
        return actual > expected;
    case ComparisonOperator::GreaterEqual:
        return actual >= expected;
    case ComparisonOperator::Less:
        return actual < expected;
    case ComparisonOperator::LessEqual:
        return actual <= expected;
    }

    return false;
}

[[nodiscard]] std::optional<std::string_view> jsonFieldValue(const analysis::IndexedLine& line,
                                                               const std::string_view field) noexcept
{
    for (const auto& fieldValue : line.jsonFieldValues)
    {
        if (fieldValue.first == field)
        {
            return fieldValue.second;
        }
    }

    return std::nullopt;
}

[[nodiscard]] bool evaluateJsonFieldComparison(const QueryNode& node, const analysis::IndexedLine& line) noexcept
{
    if (isReservedComparisonField(node.field()))
    {
        return false;
    }

    const auto actual = jsonFieldValue(line, node.field());

    if (!actual.has_value())
    {
        return false;
    }

    const QueryValue& literal = node.value();
    std::string expected;

    if (literal.kind() == QueryValue::Kind::String)
    {
        expected = literal.stringValue();
    }
    else if (literal.kind() == QueryValue::Kind::Number)
    {
        expected = std::to_string(literal.numberValue());
    }
    else
    {
        return false;
    }

    if (isOrderedComparisonOperator(node.comparisonOperator()))
    {
        if (literal.kind() != QueryValue::Kind::Number)
        {
            return false;
        }

        double actualNumber = 0.0;

        if (!tryParseJsonFieldNumber(*actual, actualNumber))
        {
            return false;
        }

        return compareNumeric(actualNumber, static_cast<double>(literal.numberValue()), node.comparisonOperator());
    }

    switch (node.comparisonOperator())
    {
    case ComparisonOperator::Equal:
        return *actual == expected;
    case ComparisonOperator::NotEqual:
        return *actual != expected;
    default:
        return false;
    }
}

[[nodiscard]] foundation::Result<bool> validateFilterSemanticsNode(const QueryNode& node) noexcept
{
    switch (node.kind())
    {
    case QueryNode::Kind::MatchAll:
        return foundation::Result<bool>(true);
    case QueryNode::Kind::Comparison:
        if (!isReservedComparisonField(node.field()) &&
            isOrderedComparisonOperator(node.comparisonOperator()) &&
            node.value().kind() == QueryValue::Kind::String)
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::InvalidArgument,
                "Ordered comparison on JSON field requires numeric literal"));
        }

        return foundation::Result<bool>(true);
    case QueryNode::Kind::FunctionCall:
        return foundation::Result<bool>(true);
    case QueryNode::Kind::And:
    {
        const auto left = validateFilterSemanticsNode(*node.left());

        if (!left)
        {
            return left;
        }

        return validateFilterSemanticsNode(*node.right());
    }
    case QueryNode::Kind::Or:
    {
        const auto left = validateFilterSemanticsNode(*node.left());

        if (!left)
        {
            return left;
        }

        return validateFilterSemanticsNode(*node.right());
    }
    case QueryNode::Kind::Not:
        return validateFilterSemanticsNode(*node.operand());
    }

    return foundation::Result<bool>(true);
}

} // namespace

foundation::Result<bool> validateFilterSemantics(const QueryNode& root) noexcept
{
    return validateFilterSemanticsNode(root);
}

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

    return evaluateJsonFieldComparison(node, line);
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
