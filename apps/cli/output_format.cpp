/**
 * @file output_format.cpp
 * @brief OutputFormat implementation.
 */

#include "output_format.hpp"

#include "foundation/string.hpp"
#include "report_format.hpp"

namespace scope::cli
{

std::optional<OutputFormat> parseOutputFormat(const std::string_view value) noexcept
{
    const auto format = reporting::parseReportFormat(value);

    if (!format)
    {
        return std::nullopt;
    }

    switch (*format)
    {
    case reporting::ReportFormat::Text:
        return OutputFormat::Text;
    case reporting::ReportFormat::Json:
        return OutputFormat::Json;
    case reporting::ReportFormat::Csv:
        return OutputFormat::Csv;
    case reporting::ReportFormat::Markdown:
        return OutputFormat::Markdown;
    case reporting::ReportFormat::Html:
        return OutputFormat::Html;
    case reporting::ReportFormat::Pdf:
        return OutputFormat::Pdf;
    }

    return std::nullopt;
}

std::string_view outputFormatName(const OutputFormat format) noexcept
{
    switch (format)
    {
    case OutputFormat::Text:
        return reporting::reportFormatName(reporting::ReportFormat::Text);
    case OutputFormat::Json:
        return reporting::reportFormatName(reporting::ReportFormat::Json);
    case OutputFormat::Csv:
        return reporting::reportFormatName(reporting::ReportFormat::Csv);
    case OutputFormat::Markdown:
        return reporting::reportFormatName(reporting::ReportFormat::Markdown);
    case OutputFormat::Html:
        return reporting::reportFormatName(reporting::ReportFormat::Html);
    case OutputFormat::Pdf:
        return reporting::reportFormatName(reporting::ReportFormat::Pdf);
    }

    return reporting::reportFormatName(reporting::ReportFormat::Text);
}

reporting::ReportFormat toReportFormat(const OutputFormat format) noexcept
{
    switch (format)
    {
    case OutputFormat::Text:
        return reporting::ReportFormat::Text;
    case OutputFormat::Json:
        return reporting::ReportFormat::Json;
    case OutputFormat::Csv:
        return reporting::ReportFormat::Csv;
    case OutputFormat::Markdown:
        return reporting::ReportFormat::Markdown;
    case OutputFormat::Html:
        return reporting::ReportFormat::Html;
    case OutputFormat::Pdf:
        return reporting::ReportFormat::Pdf;
    }

    return reporting::ReportFormat::Text;
}

} // namespace scope::cli
