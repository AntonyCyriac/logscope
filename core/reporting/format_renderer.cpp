/**
 * @file format_renderer.cpp
 * @brief Assembles report fragments into final output formats.
 */

#include "format_renderer.hpp"

#include <sstream>

#include "chart_model.hpp"
#include "minimal_pdf_writer.hpp"
#include "report_string_utils.hpp"

namespace scope::reporting
{

namespace
{

std::string assembleText(const std::vector<ReportFragment>& fragments)
{
    std::ostringstream output;

    output << "========== LOGSCOPE REPORT ==========";

    for (const ReportFragment& fragment : fragments)
    {
        if (!fragment.textBody.empty())
        {
            output << '\n' << '\n' << fragment.textBody;
        }
    }

    output << '\n' << '\n' << "=====================================";

    return output.str();
}

std::string assembleMarkdown(const std::vector<ReportFragment>& fragments)
{
    std::ostringstream output;

    output << "# LogScope Report";

    for (const ReportFragment& fragment : fragments)
    {
        if (!fragment.markdownBody.empty())
        {
            output << '\n' << '\n' << fragment.markdownBody;
        }
    }

    return output.str();
}

std::string assembleJson(const std::vector<ReportFragment>& fragments)
{
    std::ostringstream output;

    output << '{';

    bool wroteField = false;

    for (const ReportFragment& fragment : fragments)
    {
        if (fragment.jsonBody.empty() || fragment.jsonKey.empty())
        {
            continue;
        }

        if (wroteField)
        {
            output << ',';
        }

        wroteField = true;
        output << "\n  \"" << fragment.jsonKey << "\": {" << fragment.jsonBody << "\n  }";
    }

    output << "\n}";

    return output.str();
}

std::string assembleCsv(const std::vector<ReportFragment>& fragments)
{
    std::ostringstream output;

    output << "section,key,value\n";

    for (const ReportFragment& fragment : fragments)
    {
        for (const CsvRow& row : fragment.csvRows)
        {
            output << escapeCsvField(row.section) << ',' << escapeCsvField(row.key) << ','
                   << escapeCsvField(row.value) << '\n';
        }
    }

    return output.str();
}

std::string assembleHtml(const std::vector<ReportFragment>& fragments)
{
    std::ostringstream output;

    output << "<!DOCTYPE html>\n"
           << "<html lang=\"en\">\n"
           << "<head>\n"
           << "  <meta charset=\"utf-8\"/>\n"
           << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/>\n"
           << "  <title>LogScope Report</title>\n"
           << "  <style>\n"
           << "    body { font-family: system-ui, sans-serif; margin: 2rem; color: #212529; }\n"
           << "    h1 { border-bottom: 2px solid #0d6efd; padding-bottom: 0.5rem; }\n"
           << "    .report-section { margin-bottom: 2rem; }\n"
           << "    table { border-collapse: collapse; width: 100%; max-width: 48rem; }\n"
           << "    th, td { border: 1px solid #dee2e6; padding: 0.5rem 0.75rem; text-align: left; }\n"
           << "    th { background: #f8f9fa; width: 40%; }\n"
           << "    .health-verdict { font-size: 1.125rem; font-weight: 600; }\n"
           << "    .chart { margin-top: 1rem; }\n"
           << "  </style>\n"
           << "</head>\n"
           << "<body>\n"
           << "  <h1>LogScope Report</h1>\n";

    for (const ReportFragment& fragment : fragments)
    {
        if (!fragment.htmlBody.empty())
        {
            output << fragment.htmlBody;
        }
    }

    output << "</body>\n</html>\n";

    return output.str();
}

std::string trimHeadingMarkers(std::string line)
{
    if (line.rfind("---", 0) != 0)
    {
        return line;
    }

    line = line.substr(3);

    while (!line.empty() && (line.front() == ' ' || line.front() == '-'))
    {
        line.erase(line.begin());
    }

    while (!line.empty() && (line.back() == ' ' || line.back() == '-'))
    {
        line.pop_back();
    }

    return line;
}

Report assemblePdfReport(const analysis::AnalysisModel& model, const std::vector<ReportFragment>& fragments,
                         const ReportOptions& options)
{
    MinimalPdfWriter writer;

    writer.addTitle("LogScope Report");

    for (const ReportFragment& fragment : fragments)
    {
        if (fragment.textBody.empty())
        {
            continue;
        }

        std::istringstream stream(fragment.textBody);
        std::string line;
        bool firstLine = true;

        while (std::getline(stream, line))
        {
            if (line.empty())
            {
                continue;
            }

            if (firstLine && line.rfind("---", 0) == 0)
            {
                writer.addHeading(trimHeadingMarkers(line));
                firstLine = false;

                continue;
            }

            if (line.find('|') != std::string::npos && line.find('#') != std::string::npos)
            {
                continue;
            }

            firstLine = false;
            writer.addTextLine(line);
        }
    }

    if (options.includeCharts && options.sections.includes(ReportSection::Charts))
    {
        writer.addHeading("Level Chart");
        writer.addLevelChart(buildLevelBarChart(model.levelCounts()));
    }

    return Report::fromBinary(writer.finalize(), "application/pdf");
}

} // namespace

Report FormatRenderer::render(const analysis::AnalysisModel& model,
                              const std::vector<ReportFragment>& fragments,
                              const ReportOptions& options)
{
    switch (options.format)
    {
    case ReportFormat::Json:
        return Report(assembleJson(fragments));
    case ReportFormat::Csv:
        return Report(assembleCsv(fragments));
    case ReportFormat::Markdown:
        return Report(assembleMarkdown(fragments));
    case ReportFormat::Html:
        return Report(assembleHtml(fragments));
    case ReportFormat::Pdf:
        return assemblePdfReport(model, fragments, options);
    case ReportFormat::Text:
    default:
        return Report(assembleText(fragments));
    }
}

} // namespace scope::reporting
