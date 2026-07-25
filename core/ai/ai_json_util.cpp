/**
 * @file ai_json_util.cpp
 */

#include "ai_json_util.hpp"

#include <sstream>

namespace scope::ai
{

namespace
{

std::optional<std::string> readJsonString(std::string_view input, std::size_t startIndex)
{
    if (startIndex >= input.size() || input[startIndex] != '"')
    {
        return std::nullopt;
    }

    std::string value;
    bool escaping = false;

    for (std::size_t index = startIndex + 1U; index < input.size(); ++index)
    {
        const char character = input[index];

        if (escaping)
        {
            switch (character)
            {
            case '"':
                value.push_back('"');
                break;
            case '\\':
                value.push_back('\\');
                break;
            case 'n':
                value.push_back('\n');
                break;
            case 'r':
                value.push_back('\r');
                break;
            case 't':
                value.push_back('\t');
                break;
            default:
                value.push_back(character);
                break;
            }

            escaping = false;

            continue;
        }

        if (character == '\\')
        {
            escaping = true;

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

std::size_t skipWhitespace(std::string_view input, std::size_t startIndex)
{
    while (startIndex < input.size() && (input[startIndex] == ' ' || input[startIndex] == '\n' ||
                                           input[startIndex] == '\r' || input[startIndex] == '\t'))
    {
        ++startIndex;
    }

    return startIndex;
}

} // namespace

std::string escapeJsonString(const std::string_view value)
{
    std::string escaped;
    escaped.reserve(value.size());

    for (const char character : value)
    {
        switch (character)
        {
        case '"':
            escaped += "\\\"";
            break;
        case '\\':
            escaped += "\\\\";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\t':
            escaped += "\\t";
            break;
        default:
            escaped.push_back(character);
            break;
        }
    }

    return escaped;
}

std::string buildChatCompletionRequest(const std::string_view model, const std::string_view systemPrompt,
                                       const std::string_view userPrompt)
{
    std::ostringstream body;

    body << "{\"model\":\"" << escapeJsonString(model) << "\",\"messages\":["
         << "{\"role\":\"system\",\"content\":\"" << escapeJsonString(systemPrompt) << "\"},"
         << "{\"role\":\"user\",\"content\":\"" << escapeJsonString(userPrompt) << "\"}"
         << "],\"temperature\":0}";

    return body.str();
}

std::optional<std::string> extractChatCompletionContent(const std::string_view responseBody)
{
    constexpr std::string_view contentKey = "\"content\"";

    const std::size_t contentPosition = responseBody.find(contentKey);

    if (contentPosition == std::string_view::npos)
    {
        return std::nullopt;
    }

    std::size_t index = skipWhitespace(responseBody, contentPosition + contentKey.size());

    if (index >= responseBody.size() || responseBody[index] != ':')
    {
        return std::nullopt;
    }

    index = skipWhitespace(responseBody, index + 1U);

    return readJsonString(responseBody, index);
}

} // namespace scope::ai
