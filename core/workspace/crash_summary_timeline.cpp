/**
 * @file crash_summary_timeline.cpp
 */

#include "crash_summary_timeline.hpp"

#include "artifact_projector.hpp"
#include "foundation/timestamp.hpp"

namespace scope::workspace
{

namespace
{

EventSource makeEventSource(const ArtifactRecord& artifact)
{
    EventSource source;
    source.artifactId = artifact.id;
    source.artifactType = artifact.type;
    source.artifactName = artifact.name;

    return source;
}

std::optional<std::string> resolveTimestampIso(const std::string& value)
{
    const auto parsed = foundation::Timestamp::parse(value);

    if (!parsed)
    {
        return std::nullopt;
    }

    return parsed->toString();
}

struct ResolvedCrashTimestamp
{
    std::string iso;
    std::string source;
    bool approximate = false;
};

std::optional<ResolvedCrashTimestamp> resolveCrashSummaryTimestamp(const ArtifactRecord& artifact)
{
    if (!artifact.sourceModifiedAt.empty())
    {
        if (const std::optional<std::string> iso = resolveTimestampIso(artifact.sourceModifiedAt))
        {
            return ResolvedCrashTimestamp{*iso, "source_mtime", true};
        }
    }

    if (!artifact.importedAt.empty())
    {
        if (const std::optional<std::string> iso = resolveTimestampIso(artifact.importedAt))
        {
            return ResolvedCrashTimestamp{*iso, "imported_at", true};
        }
    }

    return std::nullopt;
}

void setMetadataValue(std::map<std::string, std::string>& metadata, const std::string& key,
                      const std::optional<std::string>& value)
{
    if (value.has_value())
    {
        metadata[key] = *value;
    }
}

} // namespace

std::optional<TimelineEvent> makeCrashSummaryTimelineEvent(const std::string& investigationId,
                                                           const ArtifactRecord& artifact,
                                                           const CrashReport& report)
{
    if (report.status == CrashAnalysisStatus::NotSupported)
    {
        return std::nullopt;
    }

    const std::optional<ResolvedCrashTimestamp> timestamp = resolveCrashSummaryTimestamp(artifact);

    if (!timestamp.has_value())
    {
        return std::nullopt;
    }

    TimelineEvent event;
    event.timestamp = timestamp->iso;
    event.artifactId = artifact.id;
    event.eventType = "crash.summary";
    event.source = makeEventSource(artifact);
    event.metadata["crashReportId"] = report.id;
    event.metadata["status"] = crashAnalysisStatusToString(report.status);
    event.metadata["timestampSource"] = timestamp->source;

    if (timestamp->approximate)
    {
        event.metadata["timestampApproximate"] = "true";
    }

    setMetadataValue(event.metadata, "signal", report.signal);
    setMetadataValue(event.metadata, "faultThreadId", report.faultThreadId);

    switch (report.status)
    {
    case CrashAnalysisStatus::Ready:
        event.message = report.summary.empty() ? artifact.name + ": crash summary" : report.summary;
        event.severity = report.signal.has_value() ? std::optional<std::string>("error")
                                                   : std::optional<std::string>("warning");
        break;
    case CrashAnalysisStatus::Unavailable:
        event.message = artifact.name + ": crash analysis unavailable";
        if (!report.warnings.empty())
        {
            event.message += " — " + report.warnings.front();
        }
        event.severity = "warning";
        break;
    case CrashAnalysisStatus::Failed:
        event.message = artifact.name + ": crash analysis failed";
        if (!report.summary.empty())
        {
            event.message += " — " + report.summary;
        }
        event.severity = "warning";
        break;
    case CrashAnalysisStatus::NotSupported:
        return std::nullopt;
    }

    event.id = makeTimelineEventId(investigationId, artifact.id, 0U, event.timestamp, event.eventType);

    return event;
}

} // namespace scope::workspace
