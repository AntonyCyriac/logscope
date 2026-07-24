/**
 * @file report_format.cpp
 * @brief ReportFormat implementation.
 */

#include "report_format.hpp"

#include "foundation/string.hpp"

namespace scope::reporting
{

std::optional<ReportFormat> parseReportFormat(const std::string_view value) noexcept
{
    const std::string normalized = foundation::toLower(value);

    if (normalized == "text")
    {
        return ReportFormat::Text;
    }

    if (normalized == "json")
    {
        return ReportFormat::Json;
    }

    if (normalized == "csv")
    {
        return ReportFormat::Csv;
    }

    if (normalized == "markdown" || normalized == "md")
    {
        return ReportFormat::Markdown;
    }

    if (normalized == "html")
    {
        return ReportFormat::Html;
    }

    if (normalized == "pdf")
    {
        return ReportFormat::Pdf;
    }

    return std::nullopt;
}

std::string_view reportFormatName(const ReportFormat format) noexcept
{
    switch (format)
    {
    case ReportFormat::Text:
        return "text";
    case ReportFormat::Json:
        return "json";
    case ReportFormat::Csv:
        return "csv";
    case ReportFormat::Markdown:
        return "markdown";
    case ReportFormat::Html:
        return "html";
    case ReportFormat::Pdf:
        return "pdf";
    }

    return "text";
}

} // namespace scope::reporting
