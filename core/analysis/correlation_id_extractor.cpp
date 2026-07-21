/**
 * @file correlation_id_extractor.cpp
 * @brief Correlation ID extraction implementation.
 */

#include "correlation_id_extractor.hpp"

#include "foundation/string.hpp"

namespace scope::analysis
{

namespace
{

bool extractDelimitedValue(const std::string_view line, const std::string_view marker, std::string& output) noexcept
{
    const std::string loweredLine = foundation::toLower(line);
    const std::string loweredMarker = foundation::toLower(marker);
    const std::size_t markerPosition = loweredLine.find(loweredMarker);

    if (markerPosition == std::string::npos)
    {
        return false;
    }

    std::size_t valueStart = markerPosition + marker.size();

    while (valueStart < line.size() && (line[valueStart] == '"' || line[valueStart] == '=' || line[valueStart] == ':' ||
                                        line[valueStart] == ' '))
    {
        ++valueStart;
    }

    if (valueStart >= line.size())
    {
        return false;
    }

    std::size_t valueEnd = valueStart;

    while (valueEnd < line.size() && line[valueEnd] != '"' && line[valueEnd] != ',' && line[valueEnd] != '}' &&
           line[valueEnd] != ' ')
    {
        ++valueEnd;
    }

    if (valueEnd <= valueStart)
    {
        return false;
    }

    output.assign(line.substr(valueStart, valueEnd - valueStart));

    return !output.empty();
}

} // namespace

std::string extractCorrelationId(const std::string_view line,
                               const std::string_view jsonCorrelationValue) noexcept
{
    if (!jsonCorrelationValue.empty())
    {
        return std::string(jsonCorrelationValue);
    }

    constexpr std::string_view markers[] = {"trace_id", "traceid", "correlation_id", "correlationid", "request_id",
                                          "requestid"};

    for (const std::string_view marker : markers)
    {
        std::string value;

        if (extractDelimitedValue(line, marker, value))
        {
            return value;
        }
    }

    return {};
}

} // namespace scope::analysis
