/**
 * @file artifact_projector.cpp
 */

#include "artifact_projector.hpp"

#include "artifact_handler.hpp"
#include "foundation/hash.hpp"
#include "foundation/timestamp.hpp"

#include "plain_text_field_extractor.hpp"
#include "format_parser.hpp"
#include "json_lines_parser.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>

namespace scope::workspace
{

namespace
{

std::string formatHashHex(const std::uint64_t value)
{
    std::ostringstream output;
    output << std::hex << std::setw(16) << std::setfill('0') << value;

    return output.str();
}

std::optional<foundation::Timestamp> parseArtifactImportedAt(const std::string& importedAt)
{
    const auto parsed = foundation::Timestamp::parse(importedAt);

    if (!parsed)
    {
        return std::nullopt;
    }

    return *parsed;
}

EventSource makeEventSource(const ArtifactRecord& artifact, const std::optional<std::size_t> lineNumber = std::nullopt)
{
    EventSource source;
    source.artifactId = artifact.id;
    source.artifactType = artifact.type;
    source.artifactName = artifact.name;
    source.lineNumber = lineNumber;

    return source;
}

std::string truncateMessage(const std::string& value, const std::size_t maxLength = 120U)
{
    if (value.size() <= maxLength)
    {
        return value;
    }

    return value.substr(0U, maxLength);
}

class LogArtifactProjector final : public IArtifactProjector
{
  public:
    [[nodiscard]] std::string_view artifactType() const noexcept override
    {
        return "log";
    }

    void project(const ArtifactRecord& artifact, const foundation::Path& dataPath,
                 const ArtifactProjectionContext& context, TimelineEventSink& sink) const override
    {
        std::ifstream stream(dataPath.string());

        if (!stream)
        {
            return;
        }

        std::string line;
        std::size_t lineNumber = 0U;
        std::size_t sequence = 0U;
        const analysis::FormatParser* pluginParser = context.options.lineParser;

        while (std::getline(stream, line))
        {
            ++lineNumber;

            if (context.logStats != nullptr)
            {
                ++context.logStats->linesRead;
            }

            std::optional<foundation::Timestamp> timestamp;
            std::string message;

            if (pluginParser != nullptr)
            {
                const analysis::JsonLineParseResult parsed = pluginParser->parseLine(line);

                if (parsed.outcome == analysis::JsonLineParseOutcome::Blank)
                {
                    continue;
                }

                if (parsed.outcome == analysis::JsonLineParseOutcome::Invalid || parsed.timestampValue.empty())
                {
                    if (context.logStats != nullptr)
                    {
                        ++context.logStats->linesSkippedNoTimestamp;
                    }

                    continue;
                }

                const auto timestampResult = analysis::parseLogTimestamp(parsed.timestampValue);

                if (!timestampResult.hasValue())
                {
                    if (context.logStats != nullptr)
                    {
                        ++context.logStats->linesSkippedNoTimestamp;
                    }

                    continue;
                }

                timestamp = *timestampResult;
                message = parsed.messageValue.empty() ? truncateMessage(line) : parsed.messageValue;
            }
            else
            {
                const analysis::PlainTextFields fields = analysis::PlainTextFieldExtractor::extract(line);

                if (!fields.timestamp.has_value())
                {
                    if (context.logStats != nullptr)
                    {
                        ++context.logStats->linesSkippedNoTimestamp;
                    }

                    continue;
                }

                timestamp = *fields.timestamp;
                message = fields.messageExcerpt.empty() ? truncateMessage(line) : fields.messageExcerpt;
            }

            TimelineEvent event;
            event.timestamp = timestamp->toString();
            event.artifactId = artifact.id;
            event.eventType = "log.line";
            event.message = message;
            event.source = makeEventSource(artifact, lineNumber);
            event.id = makeTimelineEventId(context.investigationId, artifact.id, sequence, event.timestamp,
                                           event.eventType);

            if (!sink.append(std::move(event)))
            {
                if (context.logStats != nullptr)
                {
                    context.logStats->eventsEmitted = sequence;
                }

                return;
            }

            ++sequence;

            if (context.logStats != nullptr)
            {
                context.logStats->eventsEmitted = sequence;
            }

            if (sequence >= context.options.maxEventsPerArtifact)
            {
                return;
            }
        }
    }
};

class NoteArtifactProjector final : public IArtifactProjector
{
  public:
    [[nodiscard]] std::string_view artifactType() const noexcept override
    {
        return "note";
    }

    void project(const ArtifactRecord& artifact, const foundation::Path& dataPath,
                 const ArtifactProjectionContext& context, TimelineEventSink& sink) const override
    {
        const std::optional<foundation::Timestamp> importedAt = parseArtifactImportedAt(artifact.importedAt);

        if (!importedAt.has_value())
        {
            return;
        }

        std::string bodyPreview;
        std::ifstream stream(dataPath.string());

        if (stream)
        {
            std::string body;
            std::getline(stream, body);
            bodyPreview = truncateMessage(body);
        }

        TimelineEvent event;
        event.timestamp = importedAt->toString();
        event.artifactId = artifact.id;
        event.eventType = "note.created";
        event.message = bodyPreview.empty() ? artifact.name : artifact.name + ": " + bodyPreview;
        event.source = makeEventSource(artifact);
        event.id =
            makeTimelineEventId(context.investigationId, artifact.id, 0U, event.timestamp, event.eventType);

        (void)sink.append(std::move(event));
    }
};

const LogArtifactProjector kLogProjector;
const NoteArtifactProjector kNoteProjector;

} // namespace

std::string makeTimelineEventId(const std::string& investigationId, const std::string& artifactId,
                                const std::size_t sequenceWithinArtifact, const std::string& timestampIso,
                                const std::string& eventType)
{
    std::ostringstream key;
    key << investigationId << '|' << artifactId << '|' << sequenceWithinArtifact << '|' << timestampIso << '|'
        << eventType;

    return formatHashHex(foundation::hashString(key.str()));
}

const IArtifactProjector* findArtifactProjector(const std::string_view type) noexcept
{
    if (type == "log")
    {
        return &kLogProjector;
    }

    if (type == "note")
    {
        return &kNoteProjector;
    }

    return nullptr;
}

} // namespace scope::workspace
