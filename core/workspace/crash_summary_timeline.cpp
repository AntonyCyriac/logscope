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

std::optional<std::string> resolveImportedAtIso(const ArtifactRecord& artifact)
{
    const auto parsed = foundation::Timestamp::parse(artifact.importedAt);

    if (!parsed)
    {
        return std::nullopt;
    }

    return parsed->toString();
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

    const std::optional<std::string> timestamp = resolveImportedAtIso(artifact);

    if (!timestamp.has_value())
    {
        return std::nullopt;
    }

    TimelineEvent event;
    event.timestamp = *timestamp;
    event.artifactId = artifact.id;
    event.eventType = "crash.summary";
    event.source = makeEventSource(artifact);
    event.metadata["crashReportId"] = report.id;
    event.metadata["status"] = crashAnalysisStatusToString(report.status);
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
