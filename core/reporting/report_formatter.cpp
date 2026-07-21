/**
 * @file report_formatter.cpp
 * @brief ReportFormatter implementation.
 */

#include "report_formatter.hpp"

#include <sstream>

#include "log_macros.hpp"

namespace scope::reporting
{

namespace
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

void appendTextSummary(std::ostringstream& output, const analysis::AnalysisModel& model)
{
    output << "--- Summary ---" << '\n';
    output << "Total log lines : " << model.totalLines() << '\n';
}

void appendTextLevelBreakdown(std::ostringstream& output, const analysis::AnalysisModel& model)
{
    const analysis::LogLevelCounts& levels = model.levelCounts();

    output << "--- Level Breakdown ---" << '\n';
    output << "Error lines     : " << levels.errorLines() << '\n';
    output << "Warning lines   : " << levels.warnLines() << '\n';
    output << "Info lines      : " << levels.infoLines() << '\n';
    output << "Other lines     : " << levels.otherLines() << '\n';
    output << "Blank lines     : " << levels.blankLines() << '\n';
}

void appendTextSourceMetadata(std::ostringstream& output, const analysis::AnalysisModel& model)
{
    output << "--- Source Metadata ---" << '\n';
    output << "Source          : " << model.sourcePath().string() << '\n';
    output << "Format          : " << analysis::logFormatName(model.format()) << '\n';
}

std::string formatTextReport(const analysis::AnalysisModel& model, const ReportSections& sections)
{
    std::ostringstream output;

    output << "========== LOGSCOPE REPORT ==========" << '\n';

    if (sections.includes(ReportSection::Summary))
    {
        output << '\n';
        appendTextSummary(output, model);
    }

    if (sections.includes(ReportSection::LevelBreakdown))
    {
        output << '\n';
        appendTextLevelBreakdown(output, model);
    }

    if (sections.includes(ReportSection::SourceMetadata))
    {
        output << '\n';
        appendTextSourceMetadata(output, model);
    }

    output << '\n' << "=====================================";

    return output.str();
}

std::string formatJsonReport(const analysis::AnalysisModel& model, const ReportSections& sections)
{
    std::ostringstream output;

    output << '{';

    bool wroteField = false;

    auto writeSeparator = [&]() {
        if (wroteField)
        {
            output << ',';
        }

        wroteField = true;
    };

    if (sections.includes(ReportSection::Summary))
    {
        writeSeparator();
        output << "\n  \"summary\": {\n"
               << "    \"totalLines\": " << model.totalLines() << "\n  }";
    }

    if (sections.includes(ReportSection::LevelBreakdown))
    {
        const analysis::LogLevelCounts& levels = model.levelCounts();

        writeSeparator();
        output << "\n  \"levelBreakdown\": {\n"
               << "    \"errorLines\": " << levels.errorLines() << ",\n"
               << "    \"warningLines\": " << levels.warnLines() << ",\n"
               << "    \"infoLines\": " << levels.infoLines() << ",\n"
               << "    \"otherLines\": " << levels.otherLines() << ",\n"
               << "    \"blankLines\": " << levels.blankLines() << "\n  }";
    }

    if (sections.includes(ReportSection::SourceMetadata))
    {
        writeSeparator();
        output << "\n  \"sourceMetadata\": {\n"
               << "    \"source\": " << escapeJsonString(model.sourcePath().string()) << ",\n"
               << "    \"format\": " << escapeJsonString(analysis::logFormatName(model.format())) << "\n  }";
    }

    output << "\n}";

    return output.str();
}

void appendCsvRow(std::ostringstream& output,
                  const std::string_view section,
                  const std::string_view key,
                  const std::uint64_t value)
{
    output << escapeCsvField(section) << ',' << escapeCsvField(key) << ',' << value << '\n';
}

void appendCsvRow(std::ostringstream& output,
                  const std::string_view section,
                  const std::string_view key,
                  const std::string_view value)
{
    output << escapeCsvField(section) << ',' << escapeCsvField(key) << ','
           << escapeCsvField(value) << '\n';
}

std::string formatCsvReport(const analysis::AnalysisModel& model, const ReportSections& sections)
{
    std::ostringstream output;

    output << "section,key,value\n";

    if (sections.includes(ReportSection::Summary))
    {
        appendCsvRow(output, "summary", "totalLines", model.totalLines());
    }

    if (sections.includes(ReportSection::LevelBreakdown))
    {
        const analysis::LogLevelCounts& levels = model.levelCounts();

        appendCsvRow(output, "levelBreakdown", "errorLines", levels.errorLines());
        appendCsvRow(output, "levelBreakdown", "warningLines", levels.warnLines());
        appendCsvRow(output, "levelBreakdown", "infoLines", levels.infoLines());
        appendCsvRow(output, "levelBreakdown", "otherLines", levels.otherLines());
        appendCsvRow(output, "levelBreakdown", "blankLines", levels.blankLines());
    }

    if (sections.includes(ReportSection::SourceMetadata))
    {
        appendCsvRow(output, "sourceMetadata", "source", model.sourcePath().string());
        appendCsvRow(output, "sourceMetadata", "format", analysis::logFormatName(model.format()));
    }

    return output.str();
}

std::string formatMarkdownReport(const analysis::AnalysisModel& model, const ReportSections& sections)
{
    std::ostringstream output;

    output << "# LogScope Report" << '\n';

    if (sections.includes(ReportSection::Summary))
    {
        output << '\n' << "## Summary" << '\n' << '\n';
        output << "| Metric | Value |" << '\n';
        output << "|--------|-------|" << '\n';
        output << "| Total log lines | " << model.totalLines() << " |" << '\n';
    }

    if (sections.includes(ReportSection::LevelBreakdown))
    {
        const analysis::LogLevelCounts& levels = model.levelCounts();

        output << '\n' << "## Level Breakdown" << '\n' << '\n';
        output << "| Level | Count |" << '\n';
        output << "|-------|-------|" << '\n';
        output << "| Error | " << levels.errorLines() << " |" << '\n';
        output << "| Warning | " << levels.warnLines() << " |" << '\n';
        output << "| Info | " << levels.infoLines() << " |" << '\n';
        output << "| Other | " << levels.otherLines() << " |" << '\n';
        output << "| Blank | " << levels.blankLines() << " |" << '\n';
    }

    if (sections.includes(ReportSection::SourceMetadata))
    {
        output << '\n' << "## Source Metadata" << '\n' << '\n';
        output << "| Field | Value |" << '\n';
        output << "|-------|-------|" << '\n';
        output << "| Source | " << model.sourcePath().string() << " |" << '\n';
        output << "| Format | " << analysis::logFormatName(model.format()) << " |" << '\n';
    }

    return output.str();
}

} // namespace

Report ReportFormatter::format(const analysis::AnalysisModel& model, const ReportOptions& options)
{
    SCOPE_LOG_INFO("reporting", "Formatting report for " + model.sourcePath().string());

    switch (options.format)
    {
    case ReportFormat::Json:
        return Report(formatJsonReport(model, options.sections));
    case ReportFormat::Csv:
        return Report(formatCsvReport(model, options.sections));
    case ReportFormat::Markdown:
        return Report(formatMarkdownReport(model, options.sections));
    case ReportFormat::Text:
    default:
        return Report(formatTextReport(model, options.sections));
    }
}

} // namespace scope::reporting
