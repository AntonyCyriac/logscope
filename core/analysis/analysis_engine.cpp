/**
 * @file analysis_engine.cpp
 * @brief AnalysisEngine implementation.
 */

#include "analysis_engine.hpp"

#include <algorithm>
#include <string>
#include <optional>
#include <vector>

#include "analysis_config.hpp"
#include "analysis_stats.hpp"
#include "field_summary.hpp"
#include "format_detector.hpp"
#include "attributed_ingest_source.hpp"
#include "discovery_census.hpp"
#include "foundation/error.hpp"
#include "hybrid_index_writer.hpp"
#include "index_fingerprint.hpp"
#include "index_reuse.hpp"
#include "index_store_factory.hpp"
#include "foundation/filesystem.hpp"
#include "foundation/stopwatch.hpp"
#include "json_lines_parser.hpp"
#include "json_lines_summary.hpp"
#include "line_index.hpp"
#include "correlation_id_extractor.hpp"
#include "log_line_classifier.hpp"
#include "log_macros.hpp"
#include "parser_registry.hpp"
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

IndexedLine buildIndexedLine(const std::uint64_t lineNumber, const DetectedLogLevel level,
                             const std::optional<foundation::Timestamp>& timestamp,
                             const std::string_view messageExcerpt, const std::string_view correlationId,
                             const std::string_view content, std::vector<std::string> topLevelKeys,
                             std::vector<std::pair<std::string, std::string>> jsonFieldValues = {},
                             const source::LineAttribution* attribution = nullptr) noexcept
{
    IndexedLine indexedLine;
    indexedLine.lineNumber = lineNumber;
    indexedLine.level = level;
    indexedLine.timestamp = timestamp;
    indexedLine.messageExcerpt = truncateExcerpt(messageExcerpt, maxMessageExcerptLength);
    indexedLine.correlationId = std::string(correlationId);
    indexedLine.contentExcerpt = truncateExcerpt(content, maxLineContentExcerptLength);
    indexedLine.topLevelKeys = std::move(topLevelKeys);
    indexedLine.jsonFieldValues = std::move(jsonFieldValues);

    if (attribution != nullptr)
    {
        indexedLine.sourceFileRelative = attribution->sourceFileRelative;
        indexedLine.fileLineNumber = attribution->fileLineNumber;
        indexedLine.lineNumber = attribution->streamLineNumber;
    }

    return indexedLine;
}

foundation::Result<bool> indexPlainTextLine(const std::uint64_t lineNumber, const std::string& line,
                                            const DetectedLogLevel level, HybridIndexWriter& writer,
                                            const source::LineAttribution* attribution = nullptr)
{
    const PlainTextFields fields = PlainTextFieldExtractor::extract(line);
    const std::string correlationId = extractCorrelationId(line, std::string_view{});

    return writer.tryAddLine(buildIndexedLine(lineNumber, level, fields.timestamp, fields.messageExcerpt, correlationId,
                                              line, {}, {}, attribution),
                             line);
}

foundation::Result<bool> indexJsonLine(const std::uint64_t lineNumber, const std::string& line,
                                       const JsonLineParseResult& parsed, const DetectedLogLevel level,
                                       HybridIndexWriter& writer, const source::LineAttribution* attribution = nullptr)
{
    std::optional<foundation::Timestamp> timestamp;

    if (!parsed.timestampValue.empty())
    {
        const auto timestampResult = parseLogTimestamp(parsed.timestampValue);

        if (timestampResult.hasValue())
        {
            timestamp = *timestampResult;
        }
    }

    const std::string correlationId = extractCorrelationId(line, parsed.correlationValue);

    return writer.tryAddLine(buildIndexedLine(lineNumber, level, timestamp, parsed.messageValue, correlationId, line,
                                              parsed.topLevelKeys, parsed.topLevelFieldValues, attribution),
                             line);
}

foundation::Result<bool> analyzePlainTextLine(const std::string& line, const std::uint64_t lineNumber,
                                              LogLevelCounts& levelCounts, FieldSummary& fieldSummary,
                                              HybridIndexWriter& writer, const source::LineAttribution* attribution)
{
    const DetectedLogLevel level = detectLogLevel(line);
    recordLevel(levelCounts, level);

    const PlainTextFields fields = PlainTextFieldExtractor::extract(line);
    recordExtractedFields(fieldSummary, fields.timestamp, fields.messageExcerpt);

    return indexPlainTextLine(lineNumber, line, level, writer, attribution);
}

foundation::Result<bool> analyzeJsonLine(const std::string& line, const std::uint64_t lineNumber,
                                         LogLevelCounts& levelCounts, JsonLinesSummary& summary,
                                         FieldSummary& fieldSummary, HybridIndexWriter& writer,
                                         const JsonFieldMapping& mapping, const source::LineAttribution* attribution)
{
    const JsonLineParseResult parsed = JsonLinesParser::parse(line, mapping);

    if (parsed.outcome == JsonLineParseOutcome::Blank)
    {
        levelCounts.recordBlank();

        return foundation::Result<bool>(true);
    }

    if (parsed.outcome == JsonLineParseOutcome::Invalid)
    {
        summary.recordParseFailure();
        levelCounts.recordOther();

        return indexPlainTextLine(lineNumber, line, DetectedLogLevel::Other, writer, attribution);
    }

    summary.recordValidLine(parsed.topLevelKeys);

    DetectedLogLevel level = DetectedLogLevel::Other;

    if (!parsed.levelValue.empty())
    {
        level = detectLogLevelFromJsonField(parsed.levelValue);
    }
    else
    {
        level = detectLogLevel(line);
    }

    recordLevel(levelCounts, level);

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

    return indexJsonLine(lineNumber, line, parsed, level, writer, attribution);
}

foundation::Result<bool> analyzeLine(const std::string& line, const std::uint64_t lineNumber, const LogFormat format,
                                   LogLevelCounts& levelCounts, JsonLinesSummary* jsonSummary,
                                   FieldSummary& fieldSummary, HybridIndexWriter& writer,
                                   const JsonFieldMapping& mapping, const FormatParser* pluginParser,
                                   const source::LineAttribution* attribution)
{
    if (pluginParser != nullptr)
    {
        const JsonLineParseResult parsed = pluginParser->parseLine(line);

        if (parsed.outcome == JsonLineParseOutcome::Blank)
        {
            levelCounts.recordBlank();

            return foundation::Result<bool>(true);
        }

        if (parsed.outcome == JsonLineParseOutcome::Invalid)
        {
            levelCounts.recordOther();

            return indexPlainTextLine(lineNumber, line, DetectedLogLevel::Other, writer, attribution);
        }

        DetectedLogLevel level = DetectedLogLevel::Other;

        if (!parsed.levelValue.empty())
        {
            level = detectLogLevelFromJsonField(parsed.levelValue);
        }
        else
        {
            level = detectLogLevel(line);
        }

        recordLevel(levelCounts, level);

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

        return indexJsonLine(lineNumber, line, parsed, level, writer, attribution);
    }

    if (format == LogFormat::JsonLines && jsonSummary != nullptr)
    {
        return analyzeJsonLine(line, lineNumber, levelCounts, *jsonSummary, fieldSummary, writer, mapping, attribution);
    }

    return analyzePlainTextLine(line, lineNumber, levelCounts, fieldSummary, writer, attribution);
}

void trackSanitizedLine(std::string& line, std::size_t& sanitizedLineCount) noexcept
{
    if (FormatDetector::sanitizeLogLine(line))
    {
        ++sanitizedLineCount;
    }
}

void warnSanitizedLines(const std::size_t sanitizedLineCount) noexcept
{
    if (sanitizedLineCount == 0U)
    {
        return;
    }

    SCOPE_LOG_WARN("analysis",
                   "Removed non-text bytes from " + std::to_string(sanitizedLineCount) +
                       " log line(s); affected content was sanitized.");
}

[[nodiscard]] std::optional<source::LineAttribution> currentLineAttribution(source::LogSource& logSource) noexcept
{
    if (const source::AttributedIngestSource* attributed = source::asAttributedLogSource(logSource))
    {
        return attributed->lastLineAttribution();
    }

    return std::nullopt;
}

void updateAnalysisAccounting(source::SourceDataset& dataset, const source::LineAttribution& attribution)
{
    source::AnalysisAccounting& accounting = dataset.analysisAccounting();

    auto fileIt = std::find_if(accounting.perFile.begin(), accounting.perFile.end(),
                               [&](const source::PerFileAnalysis& entry) {
                                   return entry.relativePath == attribution.sourceFileRelative;
                               });

    if (fileIt == accounting.perFile.end())
    {
        source::PerFileAnalysis entry;
        entry.relativePath = attribution.sourceFileRelative;
        accounting.perFile.push_back(std::move(entry));
        fileIt = accounting.perFile.end() - 1;
    }

    ++fileIt->linesIngested;
    ++fileIt->linesAnalyzed;
}

[[nodiscard]] AnalysisModel buildAnalysisModel(source::SourceDataset& dataset, const std::uint64_t totalLines,
                                               LogLevelCounts levelCounts, LogFormat resolvedFormat,
                                               std::optional<JsonLinesSummary> jsonLinesSummary,
                                               std::optional<FieldSummary> fieldSummary, LineIndex lineIndex,
                                               storage::IndexStorePtr indexStore)
{
    source::AnalysisAccounting accounting = dataset.analysisAccounting();
    accounting.streamLineCount = totalLines;
    accounting.analyzedLineCount = totalLines;

    if (fieldSummary.has_value() && resolvedFormat == LogFormat::PlainText && totalLines > 0U &&
        fieldSummary->linesWithTimestamp() == 0U)
    {
        accounting.complete = false;
        accounting.warnings.push_back(source::IngestionWarning{
            "UNKNOWN_TIMESTAMP_DIALECT",
            "One or more ingested files yielded no parseable timestamps; timestamp dialect may be unknown."});
    }

    std::optional<source::DiscoveryCensus> discovery;

    if (dataset.hasDiscoveryCensus())
    {
        discovery = dataset.discoveryCensus();
    }

    return AnalysisModel(dataset.path(),
                         totalLines,
                         levelCounts,
                         resolvedFormat,
                         std::move(jsonLinesSummary),
                         std::move(fieldSummary),
                         std::make_optional(std::move(lineIndex)),
                         indexStore,
                         std::move(discovery),
                         std::move(accounting));
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

foundation::Result<AnalysisModel> analyzeDataset(source::SourceDataset& dataset,
                                                   const AnalysisConfig& config,
                                                   AnalysisStats* statsOut)
{
    foundation::Stopwatch stopwatch;

    const auto recordStats = [&](const std::uint64_t lineCount, const std::uint64_t byteCount,
                                 const bool indexReused)
    {
        if (statsOut != nullptr)
        {
            statsOut->parseDuration = stopwatch.elapsed();
            statsOut->lineCount = lineCount;
            statsOut->byteCount = byteCount;
            statsOut->indexReused = indexReused;
        }
    };

    if (!dataset.isValid())
    {
        return foundation::Result<AnalysisModel>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Source dataset is not valid."));
    }

    const bool isStdinSource = dataset.path().string() == "-";
    std::uint64_t byteCount = 0U;
    bool accumulateReadBytes = isStdinSource;
    std::optional<storage::IndexFingerprint> sourceFingerprint;
    std::optional<storage::IndexReusePrepareResult> appendReuse;
    const FormatParser* pluginParser = nullptr;

    if (config.pluginFormatId.has_value())
    {
        pluginParser = ParserRegistry::instance().findParser(*config.pluginFormatId);
    }

    if (!isStdinSource)
    {
        const auto isFileResult = foundation::FileSystem::isFile(dataset.path());

        if (isFileResult && *isFileResult)
        {
            const auto fingerprintResult = storage::IndexFingerprint::compute(dataset.path());

            if (!fingerprintResult)
            {
                return foundation::Result<AnalysisModel>(fingerprintResult.error());
            }

            sourceFingerprint = *fingerprintResult;

            const auto fileSizeResult = foundation::FileSystem::fileSize(dataset.path());

            if (fileSizeResult)
            {
                byteCount = *fileSizeResult;
            }

            if (config.storage.reuseIndex)
            {
                const auto prepared =
                    storage::prepareIndexReuse(config.storage, *sourceFingerprint, dataset.path());

                if (!prepared)
                {
                    return foundation::Result<AnalysisModel>(prepared.error());
                }

                if (prepared->mode == storage::IndexReuseMode::Unchanged && prepared->store != nullptr)
                {
                    const storage::IndexMetadata& metadata = prepared->store->metadata();

                    recordStats(metadata.totalLines, byteCount, true);

                    return foundation::Result<AnalysisModel>(AnalysisModel(
                        dataset.path(), metadata.totalLines, LogLevelCounts{}, metadata.format, std::nullopt,
                        std::nullopt, std::nullopt, prepared->store));
                }

                if (prepared->mode == storage::IndexReuseMode::Append && prepared->store != nullptr)
                {
                    appendReuse = std::move(*prepared);
                }
            }
        }
        else
        {
            accumulateReadBytes = true;
        }
    }

    std::string line;
    source::LogSource& logSource = dataset.source();
    LogFormat resolvedFormat{LogFormat::PlainText};
    storage::IndexStorePtr persistentStore;
    std::size_t sanitizedLineCount = 0U;

    if (appendReuse.has_value())
    {
        SCOPE_LOG_INFO("analysis",
                       "Appending to persisted index from line " +
                           std::to_string(appendReuse->linesToSkip + 1U) + ": " + dataset.path().string());

        resolvedFormat = appendReuse->store->metadata().format;
        persistentStore = appendReuse->store;

        for (std::uint64_t skipped = 0U; skipped < appendReuse->linesToSkip; ++skipped)
        {
            const auto readResult = logSource.readLine(line);

            if (!readResult)
            {
                SCOPE_LOG_ERROR("analysis", readResult.error().message());

                return foundation::Result<AnalysisModel>(readResult.error());
            }

            if (!*readResult)
            {
                return foundation::Result<AnalysisModel>(foundation::Error(
                    foundation::ErrorCode::InvalidArgument,
                    "Source log ended before indexed line count while appending."));
            }

            if (accumulateReadBytes)
            {
                byteCount += line.size() + 1U;
            }
        }
    }
    else
    {
        SCOPE_LOG_INFO("analysis", "Analyzing source: " + dataset.path().string());

        struct SampleLine
        {
            std::string text;
            std::optional<source::LineAttribution> attribution;
        };

        std::vector<SampleLine> sampleLines;
        sampleLines.reserve(FormatDetector::defaultSampleLineLimit);

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

            SampleLine sample;
            sample.text = line;

            if (const std::optional<source::LineAttribution> attribution = currentLineAttribution(logSource))
            {
                sample.attribution = attribution;
            }

            sampleLines.push_back(std::move(sample));
        }

        std::vector<std::string> sampleTexts;
        sampleTexts.reserve(sampleLines.size());

        for (const SampleLine& sample : sampleLines)
        {
            sampleTexts.push_back(sample.text);
        }

        const FormatDetectionResult detection = FormatDetector::detect(sampleTexts);
        const auto formatResult = pluginParser != nullptr
                                      ? foundation::Result<LogFormat>(LogFormat::PlainText)
                                      : resolveFormat(detection, config.formatHint);

        if (!formatResult)
        {
            SCOPE_LOG_ERROR("analysis", formatResult.error().message());

            return foundation::Result<AnalysisModel>(formatResult.error());
        }

        resolvedFormat = *formatResult;

        if (config.storage.usesPersistentStore() && sourceFingerprint.has_value())
        {
            const auto created = storage::createIndexStore(config.storage, *sourceFingerprint, dataset.path(),
                                                           resolvedFormat);

            if (!created)
            {
                return foundation::Result<AnalysisModel>(created.error());
            }

            persistentStore = *created;
        }

        SCOPE_LOG_INFO("analysis",
                       "Detected log format: " + std::string(logFormatName(resolvedFormat)) +
                           " (sampled " + std::to_string(detection.sampledLines) + " lines)");

        std::uint64_t totalLines = 0;
        LogLevelCounts levelCounts;
        FieldSummary fieldSummary;
        HybridIndexWriter indexWriter(makeLineIndex(config.maxIndexedLines), config.storage, persistentStore);
        std::optional<JsonLinesSummary> jsonLinesSummary;

        if (resolvedFormat == LogFormat::JsonLines)
        {
            jsonLinesSummary.emplace();
        }

        JsonLinesSummary* jsonSummaryPointer = jsonLinesSummary.has_value() ? &*jsonLinesSummary : nullptr;

        for (SampleLine& sampleLine : sampleLines)
        {
            ++totalLines;

            if (accumulateReadBytes)
            {
                byteCount += sampleLine.text.size() + 1U;
            }

            trackSanitizedLine(sampleLine.text, sanitizedLineCount);

            const source::LineAttribution* attribution =
                sampleLine.attribution.has_value() ? &*sampleLine.attribution : nullptr;

            if (attribution != nullptr)
            {
                updateAnalysisAccounting(dataset, *attribution);
            }

            const auto analyzeResult =
                analyzeLine(sampleLine.text, totalLines, resolvedFormat, levelCounts, jsonSummaryPointer, fieldSummary,
                            indexWriter, config.jsonFieldMapping, pluginParser, attribution);

            if (!analyzeResult)
            {
                return foundation::Result<AnalysisModel>(analyzeResult.error());
            }
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

            trackSanitizedLine(line, sanitizedLineCount);

            ++totalLines;

            if (accumulateReadBytes)
            {
                byteCount += line.size() + 1U;
            }

            const std::optional<source::LineAttribution> lineAttribution = currentLineAttribution(logSource);
            const source::LineAttribution* attribution =
                lineAttribution.has_value() ? &*lineAttribution : nullptr;

            if (attribution != nullptr)
            {
                updateAnalysisAccounting(dataset, *attribution);
            }

            const auto analyzeResult =
                analyzeLine(line, totalLines, resolvedFormat, levelCounts, jsonSummaryPointer, fieldSummary, indexWriter,
                            config.jsonFieldMapping, pluginParser, attribution);

            if (!analyzeResult)
            {
                return foundation::Result<AnalysisModel>(analyzeResult.error());
            }
        }

        const auto finalizeResult = indexWriter.finalize(totalLines);

        if (!finalizeResult)
        {
            return foundation::Result<AnalysisModel>(finalizeResult.error());
        }

        if (!*finalizeResult)
        {
            return foundation::Result<AnalysisModel>(foundation::Error(
                foundation::ErrorCode::IOError, "Failed to finalize persistent index store."));
        }

        SCOPE_LOG_INFO("analysis", "Counted " + std::to_string(totalLines) + " log lines");

        warnSanitizedLines(sanitizedLineCount);

        recordStats(totalLines, byteCount, false);

        return foundation::Result<AnalysisModel>(buildAnalysisModel(dataset,
                                                                   totalLines,
                                                                   levelCounts,
                                                                   resolvedFormat,
                                                                   std::move(jsonLinesSummary),
                                                                   std::make_optional(std::move(fieldSummary)),
                                                                   indexWriter.finishLineIndex(),
                                                                   indexWriter.indexStore()));
    }

    std::uint64_t totalLines = appendReuse->linesToSkip;
    LogLevelCounts levelCounts;
    FieldSummary fieldSummary;
    HybridIndexWriter indexWriter(makeLineIndex(config.maxIndexedLines), config.storage, persistentStore);
    std::optional<JsonLinesSummary> jsonLinesSummary;

    if (resolvedFormat == LogFormat::JsonLines)
    {
        jsonLinesSummary.emplace();
    }

    JsonLinesSummary* jsonSummaryPointer = jsonLinesSummary.has_value() ? &*jsonLinesSummary : nullptr;

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

        trackSanitizedLine(line, sanitizedLineCount);

        ++totalLines;

        if (accumulateReadBytes)
        {
            byteCount += line.size() + 1U;
        }

        const std::optional<source::LineAttribution> lineAttribution = currentLineAttribution(logSource);
        const source::LineAttribution* attribution = lineAttribution.has_value() ? &*lineAttribution : nullptr;

        if (attribution != nullptr)
        {
            updateAnalysisAccounting(dataset, *attribution);
        }

        const auto analyzeResult =
            analyzeLine(line, totalLines, resolvedFormat, levelCounts, jsonSummaryPointer, fieldSummary, indexWriter,
                        config.jsonFieldMapping, pluginParser, attribution);

        if (!analyzeResult)
        {
            return foundation::Result<AnalysisModel>(analyzeResult.error());
        }
    }

    const auto finalizeResult = indexWriter.finalize(totalLines);

    if (!finalizeResult)
    {
        return foundation::Result<AnalysisModel>(finalizeResult.error());
    }

    if (!*finalizeResult)
    {
        return foundation::Result<AnalysisModel>(foundation::Error(
            foundation::ErrorCode::IOError, "Failed to finalize persistent index store."));
    }

    SCOPE_LOG_INFO("analysis", "Counted " + std::to_string(totalLines) + " log lines after append");

    warnSanitizedLines(sanitizedLineCount);

    recordStats(totalLines, byteCount, false);

    return foundation::Result<AnalysisModel>(buildAnalysisModel(dataset,
                                                                totalLines,
                                                                levelCounts,
                                                                resolvedFormat,
                                                                std::move(jsonLinesSummary),
                                                                std::make_optional(std::move(fieldSummary)),
                                                                indexWriter.finishLineIndex(),
                                                                indexWriter.indexStore()));
}

} // namespace

foundation::Result<AnalysisModel> AnalysisEngine::analyze(source::SourceDataset& dataset,
                                                          const AnalysisConfig& config,
                                                          AnalysisStats* statsOut) const
{
    return analyzeDataset(dataset, config, statsOut);
}

foundation::Result<AnalysisModel> AnalysisEngine::analyze(source::SourceDataset& dataset,
                                                          const LogFormat formatHint,
                                                          AnalysisStats* statsOut) const
{
    AnalysisConfig config = AnalysisConfig::defaults();
    config.formatHint = formatHint;

    return analyzeDataset(dataset, config, statsOut);
}

} // namespace scope::analysis
