/**
 * @file json_lines_parser.cpp
 * @brief JsonLinesParser implementation.
 */

#include "json_lines_parser.hpp"

#include <cctype>

#include "foundation/string.hpp"

namespace scope::analysis
{

namespace
{

void trimInPlace(std::string_view& input) noexcept
{
    while (!input.empty() && std::isspace(static_cast<unsigned char>(input.front())) != 0)
    {
        input.remove_prefix(1U);
    }

    while (!input.empty() && std::isspace(static_cast<unsigned char>(input.back())) != 0)
    {
        input.remove_suffix(1U);
    }
}

bool skipWhitespace(std::string_view& input) noexcept
{
    while (!input.empty() && std::isspace(static_cast<unsigned char>(input.front())) != 0)
    {
        input.remove_prefix(1U);
    }

    return !input.empty();
}

bool skipLiteral(std::string_view& input, const std::string_view literal) noexcept
{
    if (input.size() < literal.size() || input.substr(0U, literal.size()) != literal)
    {
        return false;
    }

    input.remove_prefix(literal.size());

    return true;
}

bool parseJsonString(std::string_view& input, std::string& output) noexcept
{
    if (input.empty() || input.front() != '"')
    {
        return false;
    }

    input.remove_prefix(1U);
    output.clear();

    while (!input.empty())
    {
        const char character = input.front();
        input.remove_prefix(1U);

        if (character == '"')
        {
            return true;
        }

        if (character == '\\')
        {
            if (input.empty())
            {
                return false;
            }

            const char escaped = input.front();
            input.remove_prefix(1U);

            switch (escaped)
            {
            case '"':
            case '\\':
            case '/':
                output.push_back(escaped);
                break;
            case 'b':
                output.push_back('\b');
                break;
            case 'f':
                output.push_back('\f');
                break;
            case 'n':
                output.push_back('\n');
                break;
            case 'r':
                output.push_back('\r');
                break;
            case 't':
                output.push_back('\t');
                break;
            default:
                return false;
            }

            continue;
        }

        output.push_back(character);
    }

    return false;
}

bool skipJsonString(std::string_view& input) noexcept
{
    std::string ignored;

    return parseJsonString(input, ignored);
}

bool skipNumber(std::string_view& input) noexcept
{
    if (input.empty())
    {
        return false;
    }

    if (input.front() == '-')
    {
        input.remove_prefix(1U);
    }

    if (input.empty())
    {
        return false;
    }

    if (input.front() == '0')
    {
        input.remove_prefix(1U);
    }
    else
    {
        if (std::isdigit(static_cast<unsigned char>(input.front())) == 0)
        {
            return false;
        }

        while (!input.empty() && std::isdigit(static_cast<unsigned char>(input.front())) != 0)
        {
            input.remove_prefix(1U);
        }
    }

    if (!input.empty() && input.front() == '.')
    {
        input.remove_prefix(1U);

        if (input.empty() || std::isdigit(static_cast<unsigned char>(input.front())) == 0)
        {
            return false;
        }

        while (!input.empty() && std::isdigit(static_cast<unsigned char>(input.front())) != 0)
        {
            input.remove_prefix(1U);
        }
    }

    if (!input.empty() && (input.front() == 'e' || input.front() == 'E'))
    {
        input.remove_prefix(1U);

        if (!input.empty() && (input.front() == '+' || input.front() == '-'))
        {
            input.remove_prefix(1U);
        }

        if (input.empty() || std::isdigit(static_cast<unsigned char>(input.front())) == 0)
        {
            return false;
        }

        while (!input.empty() && std::isdigit(static_cast<unsigned char>(input.front())) != 0)
        {
            input.remove_prefix(1U);
        }
    }

    return true;
}

bool skipJsonValue(std::string_view& input) noexcept;

bool skipJsonObject(std::string_view& input) noexcept
{
    if (!skipLiteral(input, "{"))
    {
        return false;
    }

    if (!skipWhitespace(input))
    {
        return false;
    }

    if (skipLiteral(input, "}"))
    {
        return true;
    }

    while (true)
    {
        if (!skipJsonString(input))
        {
            return false;
        }

        if (!skipWhitespace(input) || !skipLiteral(input, ":"))
        {
            return false;
        }

        if (!skipJsonValue(input))
        {
            return false;
        }

        if (!skipWhitespace(input))
        {
            return false;
        }

        if (skipLiteral(input, "}"))
        {
            return true;
        }

        if (!skipLiteral(input, ","))
        {
            return false;
        }

        if (!skipWhitespace(input))
        {
            return false;
        }
    }
}

bool skipJsonArray(std::string_view& input) noexcept
{
    if (!skipLiteral(input, "["))
    {
        return false;
    }

    if (!skipWhitespace(input))
    {
        return false;
    }

    if (skipLiteral(input, "]"))
    {
        return true;
    }

    while (true)
    {
        if (!skipJsonValue(input))
        {
            return false;
        }

        if (!skipWhitespace(input))
        {
            return false;
        }

        if (skipLiteral(input, "]"))
        {
            return true;
        }

        if (!skipLiteral(input, ","))
        {
            return false;
        }

        if (!skipWhitespace(input))
        {
            return false;
        }
    }
}

bool skipJsonValue(std::string_view& input) noexcept
{
    if (!skipWhitespace(input))
    {
        return false;
    }

    switch (input.front())
    {
    case '"':
        return skipJsonString(input);
    case '{':
        return skipJsonObject(input);
    case '[':
        return skipJsonArray(input);
    case 't':
        return skipLiteral(input, "true");
    case 'f':
        return skipLiteral(input, "false");
    case 'n':
        return skipLiteral(input, "null");
    default:
        return skipNumber(input);
    }
}

bool tryCaptureLevelField(std::string& levelValue, const std::string_view key, std::string_view& input) noexcept
{
    if (!levelValue.empty())
    {
        return skipJsonValue(input);
    }

    if (key == "level" || key == "severity")
    {
        std::string value;

        if (!parseJsonString(input, value))
        {
            return skipJsonValue(input);
        }

        levelValue = std::move(value);

        return true;
    }

    if (key == "log")
    {
        if (!skipWhitespace(input) || input.empty() || input.front() != '{')
        {
            return skipJsonValue(input);
        }

        input.remove_prefix(1U);

        if (!skipWhitespace(input))
        {
            return false;
        }

        if (skipLiteral(input, "}"))
        {
            return true;
        }

        while (true)
        {
            std::string nestedKey;

            if (!parseJsonString(input, nestedKey))
            {
                return false;
            }

            if (!skipWhitespace(input) || !skipLiteral(input, ":"))
            {
                return false;
            }

            if (nestedKey == "level" && levelValue.empty())
            {
                std::string value;

                if (!parseJsonString(input, value))
                {
                    return false;
                }

                levelValue = std::move(value);
            }
            else if (!skipJsonValue(input))
            {
                return false;
            }

            if (!skipWhitespace(input))
            {
                return false;
            }

            if (skipLiteral(input, "}"))
            {
                return true;
            }

            if (!skipLiteral(input, ","))
            {
                return false;
            }

            if (!skipWhitespace(input))
            {
                return false;
            }
        }
    }

    return skipJsonValue(input);
}

bool parseTopLevelObject(std::string_view& input, JsonLineParseResult& result) noexcept
{
    if (!skipLiteral(input, "{"))
    {
        return false;
    }

    if (!skipWhitespace(input))
    {
        return false;
    }

    if (skipLiteral(input, "}"))
    {
        result.outcome = JsonLineParseOutcome::Valid;

        return true;
    }

    while (true)
    {
        std::string key;

        if (!parseJsonString(input, key))
        {
            return false;
        }

        result.topLevelKeys.push_back(key);

        if (!skipWhitespace(input) || !skipLiteral(input, ":"))
        {
            return false;
        }

        if (!tryCaptureLevelField(result.levelValue, key, input))
        {
            return false;
        }

        if (!skipWhitespace(input))
        {
            return false;
        }

        if (skipLiteral(input, "}"))
        {
            result.outcome = JsonLineParseOutcome::Valid;

            return true;
        }

        if (!skipLiteral(input, ","))
        {
            return false;
        }

        if (!skipWhitespace(input))
        {
            return false;
        }
    }
}

} // namespace

JsonLineParseResult JsonLinesParser::parse(const std::string_view line) noexcept
{
    JsonLineParseResult result;

    std::string_view trimmed = line;
    trimInPlace(trimmed);

    if (trimmed.empty())
    {
        result.outcome = JsonLineParseOutcome::Blank;

        return result;
    }

    if (!parseTopLevelObject(trimmed, result))
    {
        result.outcome = JsonLineParseOutcome::Invalid;
        result.topLevelKeys.clear();
        result.levelValue.clear();

        return result;
    }

    if (!trimmed.empty())
    {
        result.outcome = JsonLineParseOutcome::Invalid;
        result.topLevelKeys.clear();
        result.levelValue.clear();

        return result;
    }

    return result;
}

DetectedLogLevel detectLogLevelFromJsonField(const std::string_view fieldValue) noexcept
{
    if (foundation::isBlank(fieldValue))
    {
        return DetectedLogLevel::Other;
    }

    const std::string lowered = foundation::toLower(fieldValue);

    if (lowered == "error" || lowered == "err" || lowered == "fatal" || lowered == "critical")
    {
        return DetectedLogLevel::Error;
    }

    if (lowered == "warn" || lowered == "warning")
    {
        return DetectedLogLevel::Warn;
    }

    if (lowered == "info" || lowered == "information" || lowered == "debug")
    {
        return DetectedLogLevel::Info;
    }

    return DetectedLogLevel::Other;
}

} // namespace scope::analysis
