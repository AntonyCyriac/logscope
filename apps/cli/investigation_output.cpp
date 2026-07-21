/**
 * @file investigation_output.cpp
 * @brief Investigation output formatting implementation.
 */

#include "investigation_output.hpp"

#include <sstream>

#include "foundation/string.hpp"
#include "log_line_classifier.hpp"
#include "search_query.hpp"

namespace scope::cli
{

namespace
{

std::string levelName(const analysis::DetectedLogLevel level)
{
    switch (level)
    {
    case analysis::DetectedLogLevel::Error:
        return "error";
    case analysis::DetectedLogLevel::Warn:
        return "warning";
    case analysis::DetectedLogLevel::Info:
        return "info";
    case analysis::DetectedLogLevel::Blank:
        return "blank";
    case analysis::DetectedLogLevel::Other:
        return "other";
    }

    return "other";
}

std::string formatTextInvestigation(const investigation::InvestigationResult& result)
{
    std::ostringstream output;

    output << "========== INVESTIGATION RESULT ==========\n";

    if (!result.searchQuerySummary.empty())
    {
        output << "Search query     : " << result.searchQuerySummary << '\n';
        output << "Search mode      : " << (result.searchMode == search::SearchMode::Regex ? "regex" : "text") << '\n';
    }

    output << "Indexed lines   : " << result.indexedLineCount << '\n';
    output << "Truncated lines : " << result.truncatedLineCount << '\n';
    output << "Matching lines  : " << result.matchingLines.size() << '\n';

    if (result.truncatedLineCount > 0U)
    {
        output << "\nWARNING: " << result.truncatedLineCount
               << " lines were not indexed; search results may be incomplete.\n";
    }

    if (!result.matchingLines.empty())
    {
        output << "\n--- Matches ---\n";

        for (const analysis::IndexedLine& line : result.matchingLines)
        {
            output << "Line " << line.lineNumber << " [" << levelName(line.level) << "] ";

            if (line.timestamp.has_value())
            {
                output << line.timestamp->toString() << ' ';
            }

            if (!line.messageExcerpt.empty())
            {
                output << line.messageExcerpt;
            }
            else
            {
                output << line.contentExcerpt;
            }

            output << '\n';
        }
    }

    const auto& repeatedErrors = result.correlations.repeatedErrors();
    const auto& sharedIds = result.correlations.sharedCorrelationIds();

    if (!repeatedErrors.empty() || !sharedIds.empty())
    {
        output << "\n--- Correlations ---\n";

        for (const investigation::CorrelationEntry& entry : repeatedErrors)
        {
            output << "Repeated error: " << entry.key << " (" << entry.count << ")\n";
        }

        for (const investigation::CorrelationEntry& entry : sharedIds)
        {
            output << "Shared ID: " << entry.key << " (" << entry.count << ")\n";
        }
    }

    output << "==========================================";

    return output.str();
}

std::string escapeJson(const std::string& value)
{
    std::string escaped;
    escaped.reserve(value.size());

    for (const char character : value)
    {
        if (character == '"' || character == '\\')
        {
            escaped.push_back('\\');
        }

        escaped.push_back(character);
    }

    return escaped;
}

std::string formatJsonInvestigation(const investigation::InvestigationResult& result)
{
    std::ostringstream output;

    output << "{\n"
           << "  \"indexedLineCount\": " << result.indexedLineCount << ",\n"
           << "  \"truncatedLineCount\": " << result.truncatedLineCount << ",\n"
           << "  \"matchingLineCount\": " << result.matchingLines.size() << ",\n"
           << "  \"searchQuery\": \"" << escapeJson(result.searchQuerySummary) << "\",\n"
           << "  \"searchMode\": \"" << (result.searchMode == search::SearchMode::Regex ? "regex" : "text") << "\",\n"
           << "  \"indexTruncated\": " << (result.truncatedLineCount > 0U ? "true" : "false") << ",\n"
           << "  \"matches\": [";

    for (std::size_t index = 0U; index < result.matchingLines.size(); ++index)
    {
        const analysis::IndexedLine& line = result.matchingLines[index];

        if (index > 0U)
        {
            output << ',';
        }

        output << "\n    {\n"
               << "      \"lineNumber\": " << line.lineNumber << ",\n"
               << "      \"level\": \"" << levelName(line.level) << "\"";

        if (line.timestamp.has_value())
        {
            output << ",\n      \"timestamp\": \"" << escapeJson(line.timestamp->toString()) << '"';
        }

        if (!line.messageExcerpt.empty())
        {
            output << ",\n      \"message\": \"" << escapeJson(line.messageExcerpt) << '"';
        }

        if (!line.correlationId.empty())
        {
            output << ",\n      \"correlationId\": \"" << escapeJson(line.correlationId) << '"';
        }

        output << "\n    }";
    }

    output << "\n  ],\n"
           << "  \"correlations\": {\n"
           << "    \"repeatedErrors\": [";

    const auto& repeatedErrors = result.correlations.repeatedErrors();

    for (std::size_t index = 0U; index < repeatedErrors.size(); ++index)
    {
        if (index > 0U)
        {
            output << ',';
        }

        output << "\n      {\"key\": \"" << escapeJson(repeatedErrors[index].key) << "\", \"count\": "
               << repeatedErrors[index].count << '}';
    }

    output << "\n    ],\n"
           << "    \"sharedCorrelationIds\": [";

    const auto& sharedIds = result.correlations.sharedCorrelationIds();

    for (std::size_t index = 0U; index < sharedIds.size(); ++index)
    {
        if (index > 0U)
        {
            output << ',';
        }

        output << "\n      {\"key\": \"" << escapeJson(sharedIds[index].key) << "\", \"count\": "
               << sharedIds[index].count << '}';
    }

    output << "\n    ]\n"
           << "  }\n"
           << '}';

    return output.str();
}

} // namespace

std::string formatInvestigationOutput(const investigation::InvestigationResult& result, const OutputFormat format)
{
    if (format == OutputFormat::Json)
    {
        return formatJsonInvestigation(result);
    }

    return formatTextInvestigation(result);
}

} // namespace scope::cli
