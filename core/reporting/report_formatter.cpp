/**
 * @file report_formatter.cpp
 * @brief ReportFormatter implementation.
 */

#include "report_formatter.hpp"

#include <sstream>

#include "field_summary.hpp"
#include "json_lines_summary.hpp"
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

void appendTextFieldSummary(std::ostringstream& output, const analysis::FieldSummary& fieldSummary)
{
    if (fieldSummary.linesWithTimestamp() > 0U)
    {
        output << "Time range start: "
               << fieldSummary.earliestTimestamp()->toString() << '\n';
        output << "Time range end  : " << fieldSummary.latestTimestamp()->toString() << '\n';
        output << "Lines w/ time   : " << fieldSummary.linesWithTimestamp() << '\n';
    }

    if (fieldSummary.linesWithMessage() > 0U)
    {
        output << "Lines w/ message: " << fieldSummary.linesWithMessage() << '\n';

        const auto topMessages = fieldSummary.topMessages();

        if (!topMessages.empty())
        {
            output << "Top messages    : ";

            for (std::size_t index = 0U; index < topMessages.size(); ++index)
            {
                if (index > 0U)
                {
                    output << ", ";
                }

                output << topMessages[index].first << " (" << topMessages[index].second << ')';
            }

            output << '\n';
        }
    }
}

void appendTextSourceMetadata(std::ostringstream& output, const analysis::AnalysisModel& model)
{
    output << "--- Source Metadata ---" << '\n';
    output << "Source          : " << model.sourcePath().string() << '\n';
    output << "Format          : " << analysis::logFormatName(model.format()) << '\n';

    if (const std::optional<analysis::JsonLinesSummary>& summary = model.jsonLinesSummary())
    {
        output << "JSON valid lines: " << summary->validLines() << '\n';
        output << "JSON parse fails: " << summary->parseFailures() << '\n';

        const auto topKeys = summary->topLevelKeys();

        if (!topKeys.empty())
        {
            output << "Top-level keys  : ";

            for (std::size_t index = 0U; index < topKeys.size(); ++index)
            {
                if (index > 0U)
                {
                    output << ", ";
                }

                output << topKeys[index].first << " (" << topKeys[index].second << ')';
            }

            output << '\n';
        }
    }

    if (const std::optional<analysis::FieldSummary>& fieldSummary = model.fieldSummary())
    {
        appendTextFieldSummary(output, *fieldSummary);
    }
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
               << "    \"format\": " << escapeJsonString(analysis::logFormatName(model.format()));

        if (const std::optional<analysis::JsonLinesSummary>& summary = model.jsonLinesSummary())
        {
            output << ",\n"
                   << "    \"jsonValidLines\": " << summary->validLines() << ",\n"
                   << "    \"jsonParseFailures\": " << summary->parseFailures() << ",\n"
                   << "    \"topLevelKeys\": {";

            const auto topKeys = summary->topLevelKeys();
            bool wroteKey = false;

            for (const auto& entry : topKeys)
            {
                if (wroteKey)
                {
                    output << ',';
                }

                wroteKey = true;
                output << "\n      " << escapeJsonString(entry.first) << ": " << entry.second;
            }

            if (wroteKey)
            {
                output << '\n';
            }

            output << "    }";
        }

        if (const std::optional<analysis::FieldSummary>& fieldSummary = model.fieldSummary())
        {
            output << ",\n"
                   << "    \"linesWithTimestamp\": " << fieldSummary->linesWithTimestamp() << ",\n"
                   << "    \"linesWithMessage\": " << fieldSummary->linesWithMessage();

            if (fieldSummary->earliestTimestamp().has_value())
            {
                output << ",\n"
                       << "    \"earliestTimestamp\": "
                       << escapeJsonString(fieldSummary->earliestTimestamp()->toString());
            }

            if (fieldSummary->latestTimestamp().has_value())
            {
                output << ",\n"
                       << "    \"latestTimestamp\": "
                       << escapeJsonString(fieldSummary->latestTimestamp()->toString());
            }

            const auto topMessages = fieldSummary->topMessages();

            if (!topMessages.empty())
            {
                output << ",\n    \"topMessages\": {";
                bool wroteMessage = false;

                for (const auto& entry : topMessages)
                {
                    if (wroteMessage)
                    {
                        output << ',';
                    }

                    wroteMessage = true;
                    output << "\n      " << escapeJsonString(entry.first) << ": " << entry.second;
                }

                if (wroteMessage)
                {
                    output << '\n';
                }

                output << "    }";
            }
        }

        output << "\n  }";
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

        if (const std::optional<analysis::JsonLinesSummary>& summary = model.jsonLinesSummary())
        {
            appendCsvRow(output, "sourceMetadata", "jsonValidLines", summary->validLines());
            appendCsvRow(output, "sourceMetadata", "jsonParseFailures", summary->parseFailures());

            for (const auto& entry : summary->topLevelKeys())
            {
                appendCsvRow(output,
                             "sourceMetadata",
                             "topLevelKey." + entry.first,
                             std::to_string(entry.second));
            }
        }

        if (const std::optional<analysis::FieldSummary>& fieldSummary = model.fieldSummary())
        {
            appendCsvRow(output, "sourceMetadata", "linesWithTimestamp", fieldSummary->linesWithTimestamp());
            appendCsvRow(output, "sourceMetadata", "linesWithMessage", fieldSummary->linesWithMessage());

            if (fieldSummary->earliestTimestamp().has_value())
            {
                appendCsvRow(output,
                             "sourceMetadata",
                             "earliestTimestamp",
                             fieldSummary->earliestTimestamp()->toString());
            }

            if (fieldSummary->latestTimestamp().has_value())
            {
                appendCsvRow(output,
                             "sourceMetadata",
                             "latestTimestamp",
                             fieldSummary->latestTimestamp()->toString());
            }

            for (const auto& entry : fieldSummary->topMessages())
            {
                appendCsvRow(output,
                             "sourceMetadata",
                             "topMessage." + entry.first,
                             std::to_string(entry.second));
            }
        }
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

        if (const std::optional<analysis::JsonLinesSummary>& summary = model.jsonLinesSummary())
        {
            output << "| JSON valid lines | " << summary->validLines() << " |" << '\n';
            output << "| JSON parse failures | " << summary->parseFailures() << " |" << '\n';

            for (const auto& entry : summary->topLevelKeys())
            {
                output << "| Key " << entry.first << " | " << entry.second << " |" << '\n';
            }
        }

        if (const std::optional<analysis::FieldSummary>& fieldSummary = model.fieldSummary())
        {
            if (fieldSummary->earliestTimestamp().has_value())
            {
                output << "| Earliest timestamp | " << fieldSummary->earliestTimestamp()->toString()
                       << " |" << '\n';
                output << "| Latest timestamp | " << fieldSummary->latestTimestamp()->toString() << " |"
                       << '\n';
            }

            output << "| Lines with timestamp | " << fieldSummary->linesWithTimestamp() << " |" << '\n';
            output << "| Lines with message | " << fieldSummary->linesWithMessage() << " |" << '\n';

            for (const auto& entry : fieldSummary->topMessages())
            {
                output << "| Message " << entry.first << " | " << entry.second << " |" << '\n';
            }
        }
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
