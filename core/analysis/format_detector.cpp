/**
 * @file format_detector.cpp
 * @brief FormatDetector implementation.
 */

#include "format_detector.hpp"

#include <cctype>

namespace scope::analysis
{

namespace
{

[[nodiscard]] bool isJsonObjectLine(std::string_view line) noexcept
{
    while (!line.empty() && std::isspace(static_cast<unsigned char>(line.front())) != 0)
    {
        line.remove_prefix(1U);
    }

    while (!line.empty() && std::isspace(static_cast<unsigned char>(line.back())) != 0)
    {
        line.remove_suffix(1U);
    }

    if (line.size() < 2U || line.front() != '{' || line.back() != '}')
    {
        return false;
    }

    // Lightweight structural check: balanced braces with a quoted key.
    int depth = 0;
    bool inString = false;
    bool escaped = false;
    bool sawQuotedKey = false;

    for (std::size_t index = 0U; index < line.size(); ++index)
    {
        const char character = line[index];

        if (inString)
        {
            if (escaped)
            {
                escaped = false;
                continue;
            }

            if (character == '\\')
            {
                escaped = true;
                continue;
            }

            if (character == '"')
            {
                inString = false;
            }

            continue;
        }

        if (character == '"')
        {
            inString = true;

            if (depth == 1)
            {
                sawQuotedKey = true;
            }

            continue;
        }

        if (character == '{')
        {
            ++depth;
            continue;
        }

        if (character == '}')
        {
            --depth;

            if (depth < 0)
            {
                return false;
            }
        }
    }

    return depth == 0 && sawQuotedKey && !inString;
}

[[nodiscard]] bool containsBinaryContent(std::string_view text) noexcept
{
    if (text.empty())
    {
        return false;
    }

    std::size_t nonPrintable = 0U;

    for (const unsigned char character : text)
    {
        if (character == '\0')
        {
            return true;
        }

        if (character == '\n' || character == '\r' || character == '\t')
        {
            continue;
        }

        if (character < 0x20U || character == 0x7FU)
        {
            ++nonPrintable;
        }
    }

    // Reject samples with a high density of control characters.
    return nonPrintable * 10U >= text.size();
}

} // namespace

FormatDetectionResult FormatDetector::detect(const std::vector<std::string>& sampleLines)
{
    FormatDetectionResult result;
    result.sampledLines = sampleLines.size();

    std::size_t nonBlank = 0U;
    std::size_t jsonObjectLines = 0U;
    std::string joined;

    for (const std::string& line : sampleLines)
    {
        joined.append(line);
        joined.push_back('\n');

        bool blank = true;

        for (const unsigned char character : line)
        {
            if (std::isspace(character) == 0)
            {
                blank = false;
                break;
            }
        }

        if (blank)
        {
            continue;
        }

        ++nonBlank;

        if (isJsonObjectLine(line))
        {
            ++jsonObjectLines;
        }
    }

    if (containsBinaryContent(joined))
    {
        result.looksBinary = true;
        result.format = LogFormat::Unknown;

        return result;
    }

    if (nonBlank == 0U)
    {
        // Empty or whitespace-only samples remain plain text (existing empty-file workflow).
        result.format = LogFormat::PlainText;

        return result;
    }

    if (jsonObjectLines * 2U >= nonBlank)
    {
        result.format = LogFormat::JsonLines;

        return result;
    }

    result.format = LogFormat::PlainText;

    return result;
}

FormatDetectionResult FormatDetector::detect(const std::string_view sampleText)
{
    std::vector<std::string> lines;
    std::size_t start = 0U;

    while (start <= sampleText.size())
    {
        const std::size_t end = sampleText.find('\n', start);
        const std::size_t count = (end == std::string_view::npos) ? (sampleText.size() - start) : (end - start);
        std::string_view line = sampleText.substr(start, count);

        if (!line.empty() && line.back() == '\r')
        {
            line.remove_suffix(1U);
        }

        lines.emplace_back(line);

        if (end == std::string_view::npos)
        {
            break;
        }

        start = end + 1U;
    }

    return detect(lines);
}

} // namespace scope::analysis
