/**
 * @file analysis_engine.cpp
 * @brief AnalysisEngine implementation.
 */

#include "analysis_engine.hpp"

#include <string>
#include <optional>
#include <vector>

#include "field_summary.hpp"
#include "format_detector.hpp"
#include "foundation/error.hpp"
#include "json_lines_parser.hpp"
#include "json_lines_summary.hpp"
#include "log_line_classifier.hpp"
#include "log_macros.hpp"
#include "plain_text_field_extractor.hpp"

namespace scope::analysis
{

namespace
{

void recordLevel(LogLevelCounts& counts, const DetectedLogLevel level) noexcept
{
    switch (level)
    {
    case DetectedLogLevel::Blank:
        counts.recordBlank();
        break;
    case DetectedLogLevel::Info:
        counts.recordInfo();
        break;
    case DetectedLogLevel::Warn:
        counts.recordWarn();
        break;
    case DetectedLogLevel::Error:
        counts.recordError();
        break;
    case DetectedLogLevel::Other:
        counts.recordOther();
        break;
    }
}

void recordExtractedFields(FieldSummary& fieldSummary, const std::optional<foundation::Timestamp>& timestamp,
                           const std::string_view message) noexcept
{
    if (timestamp.has_value())
    {
        fieldSummary.recordTimestamp(*timestamp);
    }

    if (!message.empty())
    {
        fieldSummary.recordMessage(message);
    }
}

void analyzePlainTextLine(const std::string& line, LogLevelCounts& levelCounts, FieldSummary& fieldSummary) noexcept
{
    recordLevel(levelCounts, detectLogLevel(line));

    const PlainTextFields fields = PlainTextFieldExtractor::extract(line);
    recordExtractedFields(fieldSummary, fields.timestamp, fields.messageExcerpt);
}

void analyzeJsonLine(const std::string& line, LogLevelCounts& levelCounts, JsonLinesSummary& summary,
                     FieldSummary& fieldSummary) noexcept
{
    const JsonLineParseResult parsed = JsonLinesParser::parse(line);

    if (parsed.outcome == JsonLineParseOutcome::Blank)
    {
        levelCounts.recordBlank();

        return;
    }

    if (parsed.outcome == JsonLineParseOutcome::Invalid)
    {
        summary.recordParseFailure();
        levelCounts.recordOther();

        return;
    }

    summary.recordValidLine(parsed.topLevelKeys);

    if (!parsed.levelValue.empty())
    {
        recordLevel(levelCounts, detectLogLevelFromJsonField(parsed.levelValue));
    }
    else
    {
        recordLevel(levelCounts, detectLogLevel(line));
    }

    std::optional<foundation::Timestamp> timestamp;

    if (!parsed.timestampValue.empty())
    {
        const auto timestampResult = parseLogTimestamp(parsed.timestampValue);

        if (timestampResult.hasValue())
        {
            timestamp = *timestampResult;
        }
    }

    if (!parsed.messageValue.empty())
    {
        recordExtractedFields(fieldSummary, timestamp, parsed.messageValue);
    }
    else
    {
        recordExtractedFields(fieldSummary, timestamp, std::string_view{});
    }
}

void analyzeLine(const std::string& line, const LogFormat format, LogLevelCounts& levelCounts,
                 JsonLinesSummary* jsonSummary, FieldSummary& fieldSummary) noexcept
{
    if (format == LogFormat::JsonLines && jsonSummary != nullptr)
    {
        analyzeJsonLine(line, levelCounts, *jsonSummary, fieldSummary);

        return;
    }

    analyzePlainTextLine(line, levelCounts, fieldSummary);
}

[[nodiscard]] foundation::Result<LogFormat> resolveFormat(const FormatDetectionResult& detection,
                                                          const LogFormat formatHint)
{
    if (detection.looksBinary)
    {
        return foundation::Result<LogFormat>(foundation::Error(
            foundation::ErrorCode::InvalidArgument,
            "Unsupported log format: input appears to be binary. Provide a plain-text or JSON Lines log file."));
    }

    if (formatHint == LogFormat::Auto)
    {
        if (detection.format == LogFormat::Unknown)
        {
            return foundation::Result<LogFormat>(foundation::Error(
                foundation::ErrorCode::InvalidArgument,
                "Unsupported log format: unable to identify the input. "
                "Use --log-format plain|jsonl to override detection."));
        }

        return foundation::Result<LogFormat>(detection.format);
    }

    if (formatHint == LogFormat::Unknown)
    {
        return foundation::Result<LogFormat>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Invalid log format hint."));
    }

    return foundation::Result<LogFormat>(formatHint);
}

} // namespace

foundation::Result<AnalysisModel> AnalysisEngine::analyze(source::SourceDataset& dataset,
                                                          const LogFormat formatHint) const
{
    if (!dataset.isValid())
    {
        return foundation::Result<AnalysisModel>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Source dataset is not valid."));
    }

    SCOPE_LOG_INFO("analysis", "Analyzing source: " + dataset.path().string());

    std::vector<std::string> sampleLines;
    sampleLines.reserve(FormatDetector::defaultSampleLineLimit);

    std::string line;
    source::LogSource& logSource = dataset.source();

    while (sampleLines.size() < FormatDetector::defaultSampleLineLimit)
    {
        const auto readResult = logSource.readLine(line);

        if (!readResult)
        {
            SCOPE_LOG_ERROR("analysis", readResult.error().message());

            return foundation::Result<AnalysisModel>(readResult.error());
        }

        if (!*readResult)
        {
            break;
        }

        sampleLines.push_back(line);
    }

    const FormatDetectionResult detection = FormatDetector::detect(sampleLines);
    const auto formatResult = resolveFormat(detection, formatHint);

    if (!formatResult)
    {
        SCOPE_LOG_ERROR("analysis", formatResult.error().message());

        return foundation::Result<AnalysisModel>(formatResult.error());
    }

    const LogFormat resolvedFormat = *formatResult;

    SCOPE_LOG_INFO("analysis",
                   "Detected log format: " + std::string(logFormatName(resolvedFormat)) +
                       " (sampled " + std::to_string(detection.sampledLines) + " lines)");

    std::uint64_t totalLines = 0;
    LogLevelCounts levelCounts;
    FieldSummary fieldSummary;
    std::optional<JsonLinesSummary> jsonLinesSummary;

    if (resolvedFormat == LogFormat::JsonLines)
    {
        jsonLinesSummary.emplace();
    }

    JsonLinesSummary* jsonSummaryPointer = jsonLinesSummary.has_value() ? &*jsonLinesSummary : nullptr;

    for (const std::string& sampleLine : sampleLines)
    {
        ++totalLines;
        analyzeLine(sampleLine, resolvedFormat, levelCounts, jsonSummaryPointer, fieldSummary);
    }

    while (true)
    {
        const auto readResult = logSource.readLine(line);

        if (!readResult)
        {
            SCOPE_LOG_ERROR("analysis", readResult.error().message());

            return foundation::Result<AnalysisModel>(readResult.error());
        }

        if (!*readResult)
        {
            break;
        }

        ++totalLines;
        analyzeLine(line, resolvedFormat, levelCounts, jsonSummaryPointer, fieldSummary);
    }

    SCOPE_LOG_INFO("analysis", "Counted " + std::to_string(totalLines) + " log lines");

    return foundation::Result<AnalysisModel>(AnalysisModel(dataset.path(),
                                                           totalLines,
                                                           levelCounts,
                                                           resolvedFormat,
                                                           std::move(jsonLinesSummary),
                                                           std::make_optional(std::move(fieldSummary))));
}

} // namespace scope::analysis
