/**
 * @file timeline_projector.cpp
 */

#include "timeline_projector.hpp"

#include "artifact_handler.hpp"
#include "artifact_projector.hpp"
#include "crash_summary_timeline.hpp"
#include "foundation/timestamp.hpp"

#include <algorithm>

namespace scope::workspace
{

namespace
{

struct SortableTimelineEvent
{
    TimelineEvent event;
    foundation::Timestamp instant;
    std::size_t sequenceWithinArtifact = 0U;
};

class CollectingTimelineSink final : public TimelineEventSink
{
  public:
    explicit CollectingTimelineSink(std::vector<SortableTimelineEvent>& target) : m_target(target) {}

    bool append(TimelineEvent event) override
    {
        const auto instantResult = foundation::Timestamp::parse(event.timestamp);

        if (!instantResult)
        {
            return true;
        }

        SortableTimelineEvent sortable;
        sortable.event = std::move(event);
        sortable.instant = *instantResult;
        sortable.sequenceWithinArtifact = m_sequence++;

        m_target.push_back(std::move(sortable));

        return true;
    }

  private:
    std::vector<SortableTimelineEvent>& m_target;
    std::size_t m_sequence = 0U;
};

} // namespace

foundation::Result<TimelineProjectionResult> TimelineProjector::project(const foundation::Path& investigationRoot,
                                                                          const InvestigationManifest& manifest,
                                                                          TimelineProjectionOptions options,
                                                                          const CrashReportProvider* crashProvider)
{
    TimelineProjectionResult result;
    std::vector<SortableTimelineEvent> collected;
    collected.reserve(manifest.artifacts.size() * 4U);

    ArtifactProjectionContext context;
    context.investigationId = manifest.id;
    context.investigationRoot = investigationRoot;
    context.options = options;

    for (const ArtifactRecord& artifact : manifest.artifacts)
    {
        const IArtifactProjector* projector = findArtifactProjector(artifact.type);

        if (projector == nullptr)
        {
            if (artifact.type != "pstack" && artifact.type != "core")
            {
                result.warnings.push_back("No timeline projector for artifact type: " + artifact.type);
            }

            continue;
        }

        const IArtifactHandler* handler = findArtifactHandler(artifact.type);

        if (handler == nullptr)
        {
            result.warnings.push_back("No artifact handler for artifact type: " + artifact.type);
            continue;
        }

        const auto dataPathResult = handler->resolveDataPath(investigationRoot, artifact);

        if (!dataPathResult)
        {
            result.warnings.push_back("Failed to resolve data path for artifact " + artifact.id);
            continue;
        }

        CollectingTimelineSink sink(collected);
        projector->project(artifact, *dataPathResult, context, sink);
    }

    if (crashProvider != nullptr)
    {
        for (const ArtifactRecord& artifact : manifest.artifacts)
        {
            if (artifact.type != "pstack" && artifact.type != "core")
            {
                continue;
            }

            const foundation::Result<CrashReport> reportResult = (*crashProvider)(artifact.id);

            if (!reportResult)
            {
                result.warnings.push_back("Crash summary failed for artifact " + artifact.id + ": "
                                          + reportResult.error().message());
                continue;
            }

            const std::optional<TimelineEvent> summaryEvent =
                makeCrashSummaryTimelineEvent(manifest.id, artifact, *reportResult);

            if (!summaryEvent.has_value())
            {
                continue;
            }

            const auto instantResult = foundation::Timestamp::parse(summaryEvent->timestamp);

            if (!instantResult)
            {
                result.warnings.push_back("Crash summary skipped: invalid timestamp for artifact " + artifact.id);
                continue;
            }

            SortableTimelineEvent sortable;
            sortable.event = *summaryEvent;
            sortable.instant = *instantResult;
            sortable.sequenceWithinArtifact = 0U;
            collected.push_back(std::move(sortable));
        }
    }

    const auto compareAscending = [](const SortableTimelineEvent& left, const SortableTimelineEvent& right) {
        if (left.instant != right.instant)
        {
            return left.instant < right.instant;
        }

        if (left.event.artifactId != right.event.artifactId)
        {
            return left.event.artifactId < right.event.artifactId;
        }

        return left.sequenceWithinArtifact < right.sequenceWithinArtifact;
    };

    std::sort(collected.begin(), collected.end(), compareAscending);

    if (options.order == TimelineSortOrder::Descending)
    {
        std::reverse(collected.begin(), collected.end());
    }

    result.totalMatched = collected.size();

    const std::size_t startIndex = std::min(options.offset, collected.size());
    const std::size_t endIndex = std::min(startIndex + options.limit, collected.size());

    if (endIndex < collected.size())
    {
        result.truncated = true;
    }

    result.events.reserve(endIndex - startIndex);

    for (std::size_t index = startIndex; index < endIndex; ++index)
    {
        result.events.push_back(std::move(collected[index].event));
    }

    return foundation::Result<TimelineProjectionResult>(std::move(result));
}

} // namespace scope::workspace
