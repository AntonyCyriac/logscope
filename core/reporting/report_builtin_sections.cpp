/**
 * @file report_builtin_sections.cpp
 * @brief Built-in report section renderer implementations.
 */

#include "report_section_renderer.hpp"

#include <cmath>
#include <sstream>

#include "ascii_chart_renderer.hpp"
#include "chart_model.hpp"
#include "field_summary.hpp"
#include "json_lines_summary.hpp"
#include "log_format.hpp"
#include "log_line_classifier.hpp"
#include "report_string_utils.hpp"
#include "svg_chart_renderer.hpp"

namespace scope::reporting
{

namespace
{

std::string healthVerdict(const analysis::AnalysisModel& model)
{
    const std::uint64_t totalLines = model.totalLines();

    if (totalLines == 0U)
    {
        return "No log lines analyzed";
    }

    const analysis::LogLevelCounts& levels = model.levelCounts();
    const double errorRate = (100.0 * static_cast<double>(levels.errorLines())) /
                             static_cast<double>(totalLines);
    const double warnRate = (100.0 * static_cast<double>(levels.warnLines())) /
                            static_cast<double>(totalLines);

    if (errorRate >= 10.0)
    {
        return "Critical: high error rate";
    }

    if (errorRate >= 1.0)
    {
        return "Degraded: elevated errors";
    }

    if (warnRate >= 10.0)
    {
        return "Warning: elevated warnings";
    }

    return "Healthy";
}

double percentOfTotal(const std::uint64_t count, const std::uint64_t total)
{
    if (total == 0U)
    {
        return 0.0;
    }

    return (100.0 * static_cast<double>(count)) / static_cast<double>(total);
}

void appendCsvRow(std::vector<CsvRow>& rows,
                  const std::string_view section,
                  const std::string_view key,
                  const std::uint64_t value)
{
    rows.push_back(CsvRow{std::string(section), std::string(key), std::to_string(value)});
}

void appendCsvRow(std::vector<CsvRow>& rows,
                  const std::string_view section,
                  const std::string_view key,
                  const std::string_view value)
{
    rows.push_back(CsvRow{std::string(section), std::string(key), std::string(value)});
}

class ExecutiveSummaryRenderer final : public ReportSectionRenderer
{
  public:
    [[nodiscard]] ReportSection section() const noexcept override
    {
        return ReportSection::ExecutiveSummary;
    }

    [[nodiscard]] ReportFragment render(const analysis::AnalysisModel& model,
                                        const ReportOptions& /*options*/) const override
    {
        ReportFragment fragment;
        fragment.jsonKey = "executiveSummary";

        const analysis::LogLevelCounts& levels = model.levelCounts();
        const std::uint64_t totalLines = model.totalLines();
        const std::string verdict = healthVerdict(model);
        const double errorRate = percentOfTotal(levels.errorLines(), totalLines);
        const double warnRate = percentOfTotal(levels.warnLines(), totalLines);

        {
            std::ostringstream text;
            text << "--- Executive Summary ---" << '\n';
            text << "Health verdict    : " << verdict << '\n';
            text << "Total log lines   : " << totalLines << '\n';
            text << "Error rate        : " << errorRate << "%\n";
            text << "Warning rate      : " << warnRate << "%\n";
            text << "Format            : " << analysis::logFormatName(model.format()) << '\n';

            if (const std::optional<analysis::FieldSummary>& fieldSummary = model.fieldSummary())
            {
                if (fieldSummary->earliestTimestamp().has_value())
                {
                    text << "Time range start  : " << fieldSummary->earliestTimestamp()->toString() << '\n';
                    text << "Time range end    : " << fieldSummary->latestTimestamp()->toString() << '\n';
                }
            }

            fragment.textBody = text.str();
        }

        {
            std::ostringstream markdown;
            markdown << "## Executive Summary\n\n";
            markdown << "| Metric | Value |\n";
            markdown << "|--------|-------|\n";
            markdown << "| Health verdict | " << verdict << " |\n";
            markdown << "| Total log lines | " << totalLines << " |\n";
            markdown << "| Error rate | " << errorRate << "% |\n";
            markdown << "| Warning rate | " << warnRate << "% |\n";
            markdown << "| Format | " << analysis::logFormatName(model.format()) << " |\n";

            if (const std::optional<analysis::FieldSummary>& fieldSummary = model.fieldSummary())
            {
                if (fieldSummary->earliestTimestamp().has_value())
                {
                    markdown << "| Time range start | " << fieldSummary->earliestTimestamp()->toString()
                             << " |\n";
                    markdown << "| Time range end | " << fieldSummary->latestTimestamp()->toString()
                             << " |\n";
                }
            }

            fragment.markdownBody = markdown.str();
        }

        appendCsvRow(fragment.csvRows, "executiveSummary", "healthVerdict", verdict);
        appendCsvRow(fragment.csvRows, "executiveSummary", "totalLines", totalLines);
        appendCsvRow(fragment.csvRows, "executiveSummary", "errorRatePercent", static_cast<std::uint64_t>(errorRate));
        appendCsvRow(fragment.csvRows, "executiveSummary", "warningRatePercent", static_cast<std::uint64_t>(warnRate));
        appendCsvRow(fragment.csvRows, "executiveSummary", "format", analysis::logFormatName(model.format()));

        {
            std::ostringstream json;
            json << "\n  \"healthVerdict\": " << escapeJsonString(verdict) << ",\n"
                 << "    \"totalLines\": " << totalLines << ",\n"
                 << "    \"errorRatePercent\": " << errorRate << ",\n"
                 << "    \"warningRatePercent\": " << warnRate << ",\n"
                 << "    \"format\": " << escapeJsonString(analysis::logFormatName(model.format()));

            if (const std::optional<analysis::FieldSummary>& fieldSummary = model.fieldSummary())
            {
                if (fieldSummary->earliestTimestamp().has_value())
                {
                    json << ",\n"
                         << "    \"earliestTimestamp\": "
                         << escapeJsonString(fieldSummary->earliestTimestamp()->toString()) << ",\n"
                         << "    \"latestTimestamp\": "
                         << escapeJsonString(fieldSummary->latestTimestamp()->toString());
                }
            }

            fragment.jsonBody = json.str();
        }

        {
            std::ostringstream html;
            html << "<section class=\"report-section\" id=\"executive-summary\">\n";
            html << "  <h2>Executive Summary</h2>\n";
            html << "  <p class=\"health-verdict\">" << escapeHtml(verdict) << "</p>\n";
            html << "  <table><tbody>\n";
            html << "    <tr><th>Total log lines</th><td>" << totalLines << "</td></tr>\n";
            html << "    <tr><th>Error rate</th><td>" << errorRate << "%</td></tr>\n";
            html << "    <tr><th>Warning rate</th><td>" << warnRate << "%</td></tr>\n";
            html << "    <tr><th>Format</th><td>" << escapeHtml(analysis::logFormatName(model.format()))
                 << "</td></tr>\n";
            html << "  </tbody></table>\n";
            html << "</section>\n";
            fragment.htmlBody = html.str();
        }

        return fragment;
    }
};

class SummaryRenderer final : public ReportSectionRenderer
{
  public:
    [[nodiscard]] ReportSection section() const noexcept override
    {
        return ReportSection::Summary;
    }

    [[nodiscard]] ReportFragment render(const analysis::AnalysisModel& model,
                                        const ReportOptions& /*options*/) const override
    {
        ReportFragment fragment;
        fragment.jsonKey = "summary";

        fragment.textBody = std::string("--- Summary ---\n") + "Total log lines : " +
                            std::to_string(model.totalLines()) + '\n';
        fragment.markdownBody = "## Summary\n\n| Metric | Value |\n|--------|-------|\n| Total log lines | " +
                                std::to_string(model.totalLines()) + " |\n";
        fragment.jsonBody = "\n    \"totalLines\": " + std::to_string(model.totalLines());
        appendCsvRow(fragment.csvRows, "summary", "totalLines", model.totalLines());

        std::ostringstream html;
        html << "<section class=\"report-section\" id=\"summary\">\n";
        html << "  <h2>Summary</h2>\n";
        html << "  <table><tbody><tr><th>Total log lines</th><td>" << model.totalLines()
             << "</td></tr></tbody></table>\n";
        html << "</section>\n";
        fragment.htmlBody = html.str();

        return fragment;
    }
};

class LevelBreakdownRenderer final : public ReportSectionRenderer
{
  public:
    [[nodiscard]] ReportSection section() const noexcept override
    {
        return ReportSection::LevelBreakdown;
    }

    [[nodiscard]] ReportFragment render(const analysis::AnalysisModel& model,
                                        const ReportOptions& /*options*/) const override
    {
        ReportFragment fragment;
        fragment.jsonKey = "levelBreakdown";

        const analysis::LogLevelCounts& levels = model.levelCounts();

        {
            std::ostringstream text;
            text << "--- Level Breakdown ---" << '\n';
            text << "Error lines     : " << levels.errorLines() << '\n';
            text << "Warning lines   : " << levels.warnLines() << '\n';
            text << "Info lines      : " << levels.infoLines() << '\n';
            text << "Other lines     : " << levels.otherLines() << '\n';
            text << "Blank lines     : " << levels.blankLines() << '\n';
            fragment.textBody = text.str();
        }

        {
            std::ostringstream markdown;
            markdown << "## Level Breakdown\n\n| Level | Count |\n|-------|-------|\n";
            markdown << "| Error | " << levels.errorLines() << " |\n";
            markdown << "| Warning | " << levels.warnLines() << " |\n";
            markdown << "| Info | " << levels.infoLines() << " |\n";
            markdown << "| Other | " << levels.otherLines() << " |\n";
            markdown << "| Blank | " << levels.blankLines() << " |\n";
            fragment.markdownBody = markdown.str();
        }

        fragment.jsonBody = "\n    \"errorLines\": " + std::to_string(levels.errorLines()) + ",\n"
                            "    \"warningLines\": " +
                            std::to_string(levels.warnLines()) + ",\n"
                            "    \"infoLines\": " +
                            std::to_string(levels.infoLines()) + ",\n"
                            "    \"otherLines\": " +
                            std::to_string(levels.otherLines()) + ",\n"
                            "    \"blankLines\": " +
                            std::to_string(levels.blankLines());

        appendCsvRow(fragment.csvRows, "levelBreakdown", "errorLines", levels.errorLines());
        appendCsvRow(fragment.csvRows, "levelBreakdown", "warningLines", levels.warnLines());
        appendCsvRow(fragment.csvRows, "levelBreakdown", "infoLines", levels.infoLines());
        appendCsvRow(fragment.csvRows, "levelBreakdown", "otherLines", levels.otherLines());
        appendCsvRow(fragment.csvRows, "levelBreakdown", "blankLines", levels.blankLines());

        {
            std::ostringstream html;
            html << "<section class=\"report-section\" id=\"level-breakdown\">\n";
            html << "  <h2>Level Breakdown</h2>\n";
            html << "  <table><tbody>\n";
            html << "    <tr><th>Error</th><td>" << levels.errorLines() << "</td></tr>\n";
            html << "    <tr><th>Warning</th><td>" << levels.warnLines() << "</td></tr>\n";
            html << "    <tr><th>Info</th><td>" << levels.infoLines() << "</td></tr>\n";
            html << "    <tr><th>Other</th><td>" << levels.otherLines() << "</td></tr>\n";
            html << "    <tr><th>Blank</th><td>" << levels.blankLines() << "</td></tr>\n";
            html << "  </tbody></table>\n";
            html << "</section>\n";
            fragment.htmlBody = html.str();
        }

        return fragment;
    }
};

class ErrorSummaryRenderer final : public ReportSectionRenderer
{
  public:
    [[nodiscard]] ReportSection section() const noexcept override
    {
        return ReportSection::ErrorSummary;
    }

    [[nodiscard]] ReportFragment render(const analysis::AnalysisModel& model,
                                        const ReportOptions& /*options*/) const override
    {
        ReportFragment fragment;
        fragment.jsonKey = "errorSummary";

        const analysis::LogLevelCounts& levels = model.levelCounts();
        const auto topMessages =
            model.fieldSummary() ? model.fieldSummary()->topMessages(5U) : std::vector<std::pair<std::string, std::uint64_t>>{};

        {
            std::ostringstream text;
            text << "--- Error Summary ---" << '\n';
            text << "Error lines       : " << levels.errorLines() << '\n';
            text << "Warning lines     : " << levels.warnLines() << '\n';

            if (!topMessages.empty())
            {
                text << "Top patterns      : ";

                for (std::size_t index = 0U; index < topMessages.size(); ++index)
                {
                    if (index > 0U)
                    {
                        text << ", ";
                    }

                    text << topMessages[index].first << " (" << topMessages[index].second << ')';
                }

                text << '\n';
            }

            if (const std::optional<analysis::LineIndex>& lineIndex = model.lineIndex())
            {
                std::size_t excerptCount = 0U;

                for (const analysis::IndexedLine& line : lineIndex->lines())
                {
                    if (line.level != analysis::DetectedLogLevel::Error)
                    {
                        continue;
                    }

                    text << "  [" << line.lineNumber << "] " << line.contentExcerpt << '\n';
                    ++excerptCount;

                    if (excerptCount >= 5U)
                    {
                        break;
                    }
                }
            }

            fragment.textBody = text.str();
        }

        {
            std::ostringstream markdown;
            markdown << "## Error Summary\n\n";
            markdown << "| Metric | Value |\n|--------|-------|\n";
            markdown << "| Error lines | " << levels.errorLines() << " |\n";
            markdown << "| Warning lines | " << levels.warnLines() << " |\n";

            for (const auto& entry : topMessages)
            {
                markdown << "| Pattern " << entry.first << " | " << entry.second << " |\n";
            }

            fragment.markdownBody = markdown.str();
        }

        {
            std::ostringstream json;
            json << "\n    \"errorLines\": " << levels.errorLines() << ",\n"
                 << "    \"warningLines\": " << levels.warnLines();

            if (!topMessages.empty())
            {
                json << ",\n    \"topPatterns\": {";
                bool wrotePattern = false;

                for (const auto& entry : topMessages)
                {
                    if (wrotePattern)
                    {
                        json << ',';
                    }

                    wrotePattern = true;
                    json << "\n      " << escapeJsonString(entry.first) << ": " << entry.second;
                }

                if (wrotePattern)
                {
                    json << '\n';
                }

                json << "    }";
            }

            if (const std::optional<analysis::LineIndex>& lineIndex = model.lineIndex())
            {
                json << ",\n    \"errorExcerpts\": [";
                bool wroteExcerpt = false;
                std::size_t excerptCount = 0U;

                for (const analysis::IndexedLine& line : lineIndex->lines())
                {
                    if (line.level != analysis::DetectedLogLevel::Error)
                    {
                        continue;
                    }

                    if (wroteExcerpt)
                    {
                        json << ',';
                    }

                    wroteExcerpt = true;
                    json << "\n      {\"lineNumber\": " << line.lineNumber << ", \"excerpt\": "
                         << escapeJsonString(line.contentExcerpt) << '}';
                    ++excerptCount;

                    if (excerptCount >= 5U)
                    {
                        break;
                    }
                }

                if (wroteExcerpt)
                {
                    json << '\n';
                }

                json << "    ]";
            }

            fragment.jsonBody = json.str();
        }

        appendCsvRow(fragment.csvRows, "errorSummary", "errorLines", levels.errorLines());
        appendCsvRow(fragment.csvRows, "errorSummary", "warningLines", levels.warnLines());

        for (const auto& entry : topMessages)
        {
            appendCsvRow(fragment.csvRows, "errorSummary", "topPattern." + entry.first, entry.second);
        }

        {
            std::ostringstream html;
            html << "<section class=\"report-section\" id=\"error-summary\">\n";
            html << "  <h2>Error Summary</h2>\n";
            html << "  <table><tbody>\n";
            html << "    <tr><th>Error lines</th><td>" << levels.errorLines() << "</td></tr>\n";
            html << "    <tr><th>Warning lines</th><td>" << levels.warnLines() << "</td></tr>\n";
            html << "  </tbody></table>\n";

            if (!topMessages.empty())
            {
                html << "  <h3>Top patterns</h3><ul>\n";

                for (const auto& entry : topMessages)
                {
                    html << "    <li>" << escapeHtml(entry.first) << " (" << entry.second << ")</li>\n";
                }

                html << "  </ul>\n";
            }

            html << "</section>\n";
            fragment.htmlBody = html.str();
        }

        return fragment;
    }
};

class ChartsRenderer final : public ReportSectionRenderer
{
  public:
    [[nodiscard]] ReportSection section() const noexcept override
    {
        return ReportSection::Charts;
    }

    [[nodiscard]] ReportFragment render(const analysis::AnalysisModel& model,
                                        const ReportOptions& options) const override
    {
        ReportFragment fragment;

        if (!options.includeCharts)
        {
            return fragment;
        }

        fragment.jsonKey = "charts";
        const LevelBarChart chart = buildLevelBarChart(model.levelCounts());
        const std::string asciiChart = renderAsciiLevelChart(chart);
        const std::string markdownChart = renderMarkdownLevelChart(chart);
        const std::string svgChart = renderSvgLevelChart(chart);

        {
            std::ostringstream text;
            text << "--- Charts ---" << '\n' << asciiChart;

            if (const std::optional<analysis::FieldSummary>& fieldSummary = model.fieldSummary())
            {
                const auto topMessages = fieldSummary->topMessages(3U);

                if (!topMessages.empty())
                {
                    text << "Top error patterns:\n";

                    for (const auto& entry : topMessages)
                    {
                        text << "  " << entry.first << " (" << entry.second << ")\n";
                    }
                }
            }

            fragment.textBody = text.str();
        }

        fragment.markdownBody = std::string("## Charts\n\n") + markdownChart;
        fragment.jsonBody = "\n    \"levelBarChart\": \"ascii\"";

        for (const ChartBar& bar : chart.bars)
        {
            appendCsvRow(fragment.csvRows, "charts", "level." + bar.label, bar.value);
        }

        {
            std::ostringstream html;
            html << "<section class=\"report-section\" id=\"charts\">\n";
            html << "  <h2>Charts</h2>\n";
            html << "  <div class=\"chart\">" << svgChart << "</div>\n";
            html << "</section>\n";
            fragment.htmlBody = html.str();
        }

        return fragment;
    }
};

class SourceMetadataRenderer final : public ReportSectionRenderer
{
  public:
    [[nodiscard]] ReportSection section() const noexcept override
    {
        return ReportSection::SourceMetadata;
    }

    [[nodiscard]] ReportFragment render(const analysis::AnalysisModel& model,
                                        const ReportOptions& /*options*/) const override
    {
        ReportFragment fragment;
        fragment.jsonKey = "sourceMetadata";

        {
            std::ostringstream text;
            text << "--- Source Metadata ---" << '\n';
            text << "Source          : " << model.sourcePath().string() << '\n';
            text << "Format          : " << analysis::logFormatName(model.format()) << '\n';

            if (const std::optional<analysis::JsonLinesSummary>& summary = model.jsonLinesSummary())
            {
                text << "JSON valid lines: " << summary->validLines() << '\n';
                text << "JSON parse fails: " << summary->parseFailures() << '\n';

                const auto topKeys = summary->topLevelKeys();

                if (!topKeys.empty())
                {
                    text << "Top-level keys  : ";

                    for (std::size_t index = 0U; index < topKeys.size(); ++index)
                    {
                        if (index > 0U)
                        {
                            text << ", ";
                        }

                        text << topKeys[index].first << " (" << topKeys[index].second << ')';
                    }

                    text << '\n';
                }
            }

            if (const std::optional<analysis::FieldSummary>& fieldSummary = model.fieldSummary())
            {
                if (fieldSummary->linesWithTimestamp() > 0U)
                {
                    text << "Time range start: " << fieldSummary->earliestTimestamp()->toString() << '\n';
                    text << "Time range end  : " << fieldSummary->latestTimestamp()->toString() << '\n';
                    text << "Lines w/ time   : " << fieldSummary->linesWithTimestamp() << '\n';
                }

                if (fieldSummary->linesWithMessage() > 0U)
                {
                    text << "Lines w/ message: " << fieldSummary->linesWithMessage() << '\n';

                    const auto topMessages = fieldSummary->topMessages();

                    if (!topMessages.empty())
                    {
                        text << "Top messages    : ";

                        for (std::size_t index = 0U; index < topMessages.size(); ++index)
                        {
                            if (index > 0U)
                            {
                                text << ", ";
                            }

                            text << topMessages[index].first << " (" << topMessages[index].second << ')';
                        }

                        text << '\n';
                    }
                }
            }

            fragment.textBody = text.str();
        }

        {
            std::ostringstream markdown;
            markdown << "## Source Metadata\n\n| Field | Value |\n|-------|-------|\n";
            markdown << "| Source | " << model.sourcePath().string() << " |\n";
            markdown << "| Format | " << analysis::logFormatName(model.format()) << " |\n";

            if (const std::optional<analysis::JsonLinesSummary>& summary = model.jsonLinesSummary())
            {
                markdown << "| JSON valid lines | " << summary->validLines() << " |\n";
                markdown << "| JSON parse failures | " << summary->parseFailures() << " |\n";

                for (const auto& entry : summary->topLevelKeys())
                {
                    markdown << "| Key " << entry.first << " | " << entry.second << " |\n";
                }
            }

            if (const std::optional<analysis::FieldSummary>& fieldSummary = model.fieldSummary())
            {
                if (fieldSummary->earliestTimestamp().has_value())
                {
                    markdown << "| Earliest timestamp | " << fieldSummary->earliestTimestamp()->toString()
                             << " |\n";
                    markdown << "| Latest timestamp | " << fieldSummary->latestTimestamp()->toString()
                             << " |\n";
                }

                markdown << "| Lines with timestamp | " << fieldSummary->linesWithTimestamp() << " |\n";
                markdown << "| Lines with message | " << fieldSummary->linesWithMessage() << " |\n";

                for (const auto& entry : fieldSummary->topMessages())
                {
                    markdown << "| Message " << entry.first << " | " << entry.second << " |\n";
                }
            }

            fragment.markdownBody = markdown.str();
        }

        {
            std::ostringstream json;
            json << "\n    \"source\": " << escapeJsonString(model.sourcePath().string()) << ",\n"
                 << "    \"format\": " << escapeJsonString(analysis::logFormatName(model.format()));

            if (const std::optional<analysis::JsonLinesSummary>& summary = model.jsonLinesSummary())
            {
                json << ",\n"
                     << "    \"jsonValidLines\": " << summary->validLines() << ",\n"
                     << "    \"jsonParseFailures\": " << summary->parseFailures() << ",\n"
                     << "    \"topLevelKeys\": {";

                const auto topKeys = summary->topLevelKeys();
                bool wroteKey = false;

                for (const auto& entry : topKeys)
                {
                    if (wroteKey)
                    {
                        json << ',';
                    }

                    wroteKey = true;
                    json << "\n      " << escapeJsonString(entry.first) << ": " << entry.second;
                }

                if (wroteKey)
                {
                    json << '\n';
                }

                json << "    }";
            }

            if (const std::optional<analysis::FieldSummary>& fieldSummary = model.fieldSummary())
            {
                json << ",\n"
                     << "    \"linesWithTimestamp\": " << fieldSummary->linesWithTimestamp() << ",\n"
                     << "    \"linesWithMessage\": " << fieldSummary->linesWithMessage();

                if (fieldSummary->earliestTimestamp().has_value())
                {
                    json << ",\n"
                         << "    \"earliestTimestamp\": "
                         << escapeJsonString(fieldSummary->earliestTimestamp()->toString());
                }

                if (fieldSummary->latestTimestamp().has_value())
                {
                    json << ",\n"
                         << "    \"latestTimestamp\": "
                         << escapeJsonString(fieldSummary->latestTimestamp()->toString());
                }

                const auto topMessages = fieldSummary->topMessages();

                if (!topMessages.empty())
                {
                    json << ",\n    \"topMessages\": {";
                    bool wroteMessage = false;

                    for (const auto& entry : topMessages)
                    {
                        if (wroteMessage)
                        {
                            json << ',';
                        }

                        wroteMessage = true;
                        json << "\n      " << escapeJsonString(entry.first) << ": " << entry.second;
                    }

                    if (wroteMessage)
                    {
                        json << '\n';
                    }

                    json << "    }";
                }
            }

            fragment.jsonBody = json.str();
        }

        appendCsvRow(fragment.csvRows, "sourceMetadata", "source", model.sourcePath().string());
        appendCsvRow(fragment.csvRows, "sourceMetadata", "format", analysis::logFormatName(model.format()));

        if (const std::optional<analysis::JsonLinesSummary>& summary = model.jsonLinesSummary())
        {
            appendCsvRow(fragment.csvRows, "sourceMetadata", "jsonValidLines", summary->validLines());
            appendCsvRow(fragment.csvRows, "sourceMetadata", "jsonParseFailures", summary->parseFailures());

            for (const auto& entry : summary->topLevelKeys())
            {
                appendCsvRow(fragment.csvRows,
                             "sourceMetadata",
                             "topLevelKey." + entry.first,
                             std::to_string(entry.second));
            }
        }

        if (const std::optional<analysis::FieldSummary>& fieldSummary = model.fieldSummary())
        {
            appendCsvRow(fragment.csvRows, "sourceMetadata", "linesWithTimestamp", fieldSummary->linesWithTimestamp());
            appendCsvRow(fragment.csvRows, "sourceMetadata", "linesWithMessage", fieldSummary->linesWithMessage());

            if (fieldSummary->earliestTimestamp().has_value())
            {
                appendCsvRow(fragment.csvRows,
                             "sourceMetadata",
                             "earliestTimestamp",
                             fieldSummary->earliestTimestamp()->toString());
            }

            if (fieldSummary->latestTimestamp().has_value())
            {
                appendCsvRow(fragment.csvRows,
                             "sourceMetadata",
                             "latestTimestamp",
                             fieldSummary->latestTimestamp()->toString());
            }

            for (const auto& entry : fieldSummary->topMessages())
            {
                appendCsvRow(fragment.csvRows,
                             "sourceMetadata",
                             "topMessage." + entry.first,
                             std::to_string(entry.second));
            }
        }

        {
            std::ostringstream html;
            html << "<section class=\"report-section\" id=\"source-metadata\">\n";
            html << "  <h2>Source Metadata</h2>\n";
            html << "  <table><tbody>\n";
            html << "    <tr><th>Source</th><td>" << escapeHtml(model.sourcePath().string()) << "</td></tr>\n";
            html << "    <tr><th>Format</th><td>" << escapeHtml(analysis::logFormatName(model.format()))
                 << "</td></tr>\n";
            html << "  </tbody></table>\n";
            html << "</section>\n";
            fragment.htmlBody = html.str();
        }

        return fragment;
    }
};

void registerBuiltInSectionRenderersImpl(ReportSectionRegistry& registry)
{
    registry.registerRenderer(std::make_unique<ExecutiveSummaryRenderer>());
    registry.registerRenderer(std::make_unique<SummaryRenderer>());
    registry.registerRenderer(std::make_unique<LevelBreakdownRenderer>());
    registry.registerRenderer(std::make_unique<ErrorSummaryRenderer>());
    registry.registerRenderer(std::make_unique<ChartsRenderer>());
    registry.registerRenderer(std::make_unique<SourceMetadataRenderer>());
}

} // namespace

void registerBuiltInSectionRenderers(ReportSectionRegistry& registry)
{
    registerBuiltInSectionRenderersImpl(registry);
}

} // namespace scope::reporting
