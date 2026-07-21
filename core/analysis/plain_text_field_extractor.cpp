/**
 * @file plain_text_field_extractor.cpp
 * @brief PlainTextFieldExtractor implementation.
 */

#include "plain_text_field_extractor.hpp"

#include <cctype>
#include <string>

#include "field_summary.hpp"
#include "foundation/error.hpp"
#include "foundation/string.hpp"

namespace scope::analysis
{

namespace
{

bool isDigitAt(const std::string_view value, const std::size_t index) noexcept
{
    return index < value.size() && std::isdigit(static_cast<unsigned char>(value[index])) != 0;
}

bool hasPlainDateTimePrefix(const std::string_view value) noexcept
{
    if (value.size() < 19U)
    {
        return false;
    }

    return value[4] == '-' && value[7] == '-' && (value[10] == ' ' || value[10] == 'T') && value[13] == ':' &&
           value[16] == ':' && isDigitAt(value, 0U) && isDigitAt(value, 18U);
}

std::string normalizeTimestampText(std::string_view value) noexcept
{
    if (value.size() >= 19U && value[10] == ' ')
    {
        std::string normalized;
        normalized.reserve(19U);
        normalized.append(value.substr(0U, 10U));
        normalized.push_back('T');
        normalized.append(value.substr(11U, 8U));

        return normalized;
    }

    return std::string(value);
}

std::string_view stripLeadingLevelToken(std::string_view remainder) noexcept
{
    while (!remainder.empty() && std::isspace(static_cast<unsigned char>(remainder.front())) != 0)
    {
        remainder.remove_prefix(1U);
    }

    const std::size_t space = remainder.find(' ');

    if (space == std::string_view::npos || space > 12U)
    {
        return remainder;
    }

    const std::string token = foundation::toLower(remainder.substr(0U, space));

    if (token == "info" || token == "information" || token == "warn" || token == "warning" || token == "error" ||
        token == "err" || token == "fatal" || token == "debug" || token == "trace")
    {
        remainder.remove_prefix(space + 1U);
    }

    return remainder;
}

} // namespace

foundation::Result<foundation::Timestamp> parseLogTimestamp(const std::string_view value) noexcept
{
    if (foundation::isBlank(value))
    {
        return foundation::Result<foundation::Timestamp>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Timestamp value is blank."));
    }

    std::string_view trimmed = value;

    while (!trimmed.empty() && std::isspace(static_cast<unsigned char>(trimmed.front())) != 0)
    {
        trimmed.remove_prefix(1U);
    }

    while (!trimmed.empty())
    {
        const char trailing = trimmed.back();

        if (trailing == 'Z' || trailing == 'z' || std::isspace(static_cast<unsigned char>(trailing)) != 0)
        {
            trimmed.remove_suffix(1U);
            continue;
        }

        break;
    }

    const std::string normalized = normalizeTimestampText(trimmed);

    return foundation::Timestamp::parse(normalized);
}

PlainTextFields PlainTextFieldExtractor::extract(const std::string_view line) noexcept
{
    PlainTextFields fields;

    if (foundation::isBlank(line))
    {
        return fields;
    }

    if (!hasPlainDateTimePrefix(line))
    {
        fields.messageExcerpt = normalizeMessageExcerpt(line);

        return fields;
    }

    const auto timestampResult = parseLogTimestamp(line.substr(0U, 19U));

    if (timestampResult.hasValue())
    {
        fields.timestamp = *timestampResult;
    }

    std::string_view remainder = line.substr(19U);
    remainder = stripLeadingLevelToken(remainder);
    fields.messageExcerpt = normalizeMessageExcerpt(remainder);

    return fields;
}

} // namespace scope::analysis
