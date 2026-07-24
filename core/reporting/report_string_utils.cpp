/**
 * @file report_string_utils.cpp
 * @brief Shared escaping helpers for report formatters.
 */

#include "report_string_utils.hpp"

#include <sstream>

namespace scope::reporting
{

std::string escapeJsonString(const std::string_view value)
{
    std::ostringstream output;

    output << '"';

    for (const char character : value)
    {
        switch (character)
        {
        case '"':
            output << "\\\"";
            break;
        case '\\':
            output << "\\\\";
            break;
        case '\n':
            output << "\\n";
            break;
        case '\r':
            output << "\\r";
            break;
        case '\t':
            output << "\\t";
            break;
        default:
            output << character;
            break;
        }
    }

    output << '"';

    return output.str();
}

std::string escapeCsvField(const std::string_view value)
{
    bool requiresQuotes = false;

    for (const char character : value)
    {
        if (character == '"' || character == ',' || character == '\n' || character == '\r')
        {
            requiresQuotes = true;

            break;
        }
    }

    if (!requiresQuotes)
    {
        return std::string(value);
    }

    std::ostringstream output;

    output << '"';

    for (const char character : value)
    {
        if (character == '"')
        {
            output << "\"\"";
        }
        else
        {
            output << character;
        }
    }

    output << '"';

    return output.str();
}

std::string escapeHtml(const std::string_view value)
{
    std::ostringstream output;

    for (const char character : value)
    {
        switch (character)
        {
        case '&':
            output << "&amp;";
            break;
        case '<':
            output << "&lt;";
            break;
        case '>':
            output << "&gt;";
            break;
        case '"':
            output << "&quot;";
            break;
        default:
            output << character;
            break;
        }
    }

    return output.str();
}

} // namespace scope::reporting
