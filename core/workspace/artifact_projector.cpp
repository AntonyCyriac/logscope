/**
 * @file artifact_projector.cpp
 */

#include "artifact_projector.hpp"

#include "artifact_handler.hpp"
#include "foundation/hash.hpp"
#include "foundation/timestamp.hpp"

#include "plain_text_field_extractor.hpp"

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

        while (std::getline(stream, line))
        {
            ++lineNumber;

            const analysis::PlainTextFields fields = analysis::PlainTextFieldExtractor::extract(line);

            if (!fields.timestamp.has_value())
            {
                continue;
            }

            TimelineEvent event;
            event.timestamp = fields.timestamp->toString();
            event.artifactId = artifact.id;
            event.eventType = "log.line";
            event.message = fields.messageExcerpt.empty() ? truncateMessage(line) : fields.messageExcerpt;
            event.source = makeEventSource(artifact, lineNumber);
            event.id = makeTimelineEventId(context.investigationId, artifact.id, sequence, event.timestamp,
                                           event.eventType);

            if (!sink.append(std::move(event)))
            {
                return;
            }

            ++sequence;

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

class AttachedArtifactProjector final : public IArtifactProjector
{
  public:
    explicit AttachedArtifactProjector(std::string type, std::string messagePrefix)
        : m_type(std::move(type)), m_messagePrefix(std::move(messagePrefix))
    {
    }

    [[nodiscard]] std::string_view artifactType() const noexcept override
    {
        return m_type;
    }

    void project(const ArtifactRecord& artifact, const foundation::Path& dataPath,
                 const ArtifactProjectionContext& context, TimelineEventSink& sink) const override
    {
        (void)dataPath;

        const std::optional<foundation::Timestamp> importedAt = parseArtifactImportedAt(artifact.importedAt);

        if (!importedAt.has_value())
        {
            return;
        }

        std::string message = m_messagePrefix;

        if (const auto sizeIt = artifact.metadata.find("sizeBytes"); sizeIt != artifact.metadata.end())
        {
            message += " (" + sizeIt->second + " bytes)";
        }

        TimelineEvent event;
        event.timestamp = importedAt->toString();
        event.artifactId = artifact.id;
        event.eventType = "artifact.attached";
        event.message = message;
        event.source = makeEventSource(artifact);
        event.metadata = artifact.metadata;
        event.id =
            makeTimelineEventId(context.investigationId, artifact.id, 0U, event.timestamp, event.eventType);

        (void)sink.append(std::move(event));
    }

  private:
    std::string m_type;
    std::string m_messagePrefix;
};

const LogArtifactProjector kLogProjector;
const NoteArtifactProjector kNoteProjector;
const AttachedArtifactProjector kPstackProjector("pstack", "Pstack attached");
const AttachedArtifactProjector kCoreProjector("core", "Core dump attached");

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

    if (type == "pstack")
    {
        return &kPstackProjector;
    }

    if (type == "core")
    {
        return &kCoreProjector;
    }

    return nullptr;
}

} // namespace scope::workspace
