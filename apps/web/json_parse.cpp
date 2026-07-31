/**
 * @file json_parse.cpp
 */

#include "json_parse.hpp"

#include <cctype>

namespace scope::web
{

namespace
{

std::string_view skipWhitespace(std::string_view value)
{
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front())) != 0)
    {
        value.remove_prefix(1U);
    }

    return value;
}

std::optional<std::size_t> findKey(std::string_view body, const std::string_view key)
{
    const std::string quotedKey = '"' + std::string(key) + '"';
    const std::size_t position = body.find(quotedKey);

    if (position == std::string::npos)
    {
        return std::nullopt;
    }

    std::string_view remainder = body.substr(position + quotedKey.size());
    remainder = skipWhitespace(remainder);

    if (!remainder.empty() && remainder.front() == ':')
    {
        remainder.remove_prefix(1U);
    }

    remainder = skipWhitespace(remainder);

    return position + quotedKey.size();
}

} // namespace

std::optional<std::string> jsonStringField(const std::string_view body, const std::string_view key)
{
    const std::size_t keyPosition = body.find('"' + std::string(key) + '"');

    if (keyPosition == std::string::npos)
    {
        return std::nullopt;
    }

    std::string_view remainder = body.substr(keyPosition);
    const std::size_t colon = remainder.find(':');

    if (colon == std::string::npos)
    {
        return std::nullopt;
    }

    remainder = skipWhitespace(remainder.substr(colon + 1U));

    if (remainder.empty() || remainder.front() != '"')
    {
        return std::nullopt;
    }

    remainder.remove_prefix(1U);

    std::string value;

    for (std::size_t index = 0U; index < remainder.size(); ++index)
    {
        const char character = remainder[index];

        if (character == '\\' && index + 1U < remainder.size())
        {
            value.push_back(remainder[index + 1U]);
            ++index;

            continue;
        }

        if (character == '"')
        {
            return value;
        }

        value.push_back(character);
    }

    return std::nullopt;
}

std::optional<bool> jsonBoolField(const std::string_view body, const std::string_view key)
{
    const std::size_t keyPosition = body.find('"' + std::string(key) + '"');

    if (keyPosition == std::string::npos)
    {
        return std::nullopt;
    }

    std::string_view remainder = body.substr(keyPosition);
    const std::size_t colon = remainder.find(':');

    if (colon == std::string::npos)
    {
        return std::nullopt;
    }

    remainder = skipWhitespace(remainder.substr(colon + 1U));

    if (remainder.rfind("true", 0) == 0)
    {
        return true;
    }

    if (remainder.rfind("false", 0) == 0)
    {
        return false;
    }

    return std::nullopt;
}

std::optional<std::int64_t> jsonIntField(const std::string_view body, const std::string_view key)
{
    const std::size_t keyPosition = body.find('"' + std::string(key) + '"');

    if (keyPosition == std::string::npos)
    {
        return std::nullopt;
    }

    std::string_view remainder = body.substr(keyPosition);
    const std::size_t colon = remainder.find(':');

    if (colon == std::string::npos)
    {
        return std::nullopt;
    }

    remainder = skipWhitespace(remainder.substr(colon + 1U));

    std::int64_t value = 0;
    bool negative = false;

    if (!remainder.empty() && remainder.front() == '-')
    {
        negative = true;
        remainder.remove_prefix(1U);
    }

    if (remainder.empty() || !std::isdigit(static_cast<unsigned char>(remainder.front())))
    {
        return std::nullopt;
    }

    for (const char character : remainder)
    {
        if (!std::isdigit(static_cast<unsigned char>(character)))
        {
            break;
        }

        value = value * 10 + static_cast<std::int64_t>(character - '0');
    }

    return negative ? -value : value;
}

} // namespace scope::web
