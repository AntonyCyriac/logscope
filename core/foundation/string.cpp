/**
 * @file string.cpp
 * @brief String utility implementation.
 */

#include "string.hpp"

#include <cctype>

namespace scope::foundation
{

namespace
{

bool isSpace(char character)
{
    return std::isspace(static_cast<unsigned char>(character)) != 0;
}

} // namespace

bool isBlank(std::string_view value) noexcept
{
    for (char character : value)
    {
        if (!isSpace(character))
        {
            return false;
        }
    }

    return true;
}

std::string trimLeft(std::string_view value)
{
    std::size_t start = 0;

    while (start < value.size() && isSpace(value[start]))
    {
        ++start;
    }

    return std::string(value.substr(start));
}

std::string trimRight(std::string_view value)
{
    std::size_t end = value.size();

    while (end > 0 && isSpace(value[end - 1]))
    {
        --end;
    }

    return std::string(value.substr(0, end));
}

std::string trim(std::string_view value)
{
    return trimLeft(trimRight(value));
}

std::string toLower(std::string_view value)
{
    std::string lowered(value);

    for (char& character : lowered)
    {
        character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    }

    return lowered;
}

std::string toUpper(std::string_view value)
{
    std::string upper(value);

    for (char& character : upper)
    {
        character = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
    }

    return upper;
}

bool startsWith(std::string_view value, std::string_view prefix) noexcept
{
    if (prefix.size() > value.size())
    {
        return false;
    }

    return value.substr(0, prefix.size()) == prefix;
}

bool endsWith(std::string_view value, std::string_view suffix) noexcept
{
    if (suffix.size() > value.size())
    {
        return false;
    }

    return value.substr(value.size() - suffix.size()) == suffix;
}

std::vector<std::string> split(std::string_view value, char delimiter)
{
    std::vector<std::string> parts;

    std::size_t start = 0;

    while (start <= value.size())
    {
        const std::size_t end = value.find(delimiter, start);

        if (end == std::string_view::npos)
        {
            parts.emplace_back(value.substr(start));

            break;
        }

        parts.emplace_back(value.substr(start, end - start));
        start = end + 1;
    }

    return parts;
}

} // namespace scope::foundation
