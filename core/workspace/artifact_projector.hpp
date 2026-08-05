/**
 * @file artifact_projector.hpp
 * @brief Projects investigation artifacts into timeline events (Story 3 / v2.5.0).
 */

#pragma once

#include <string_view>

#include "artifact_record.hpp"
#include "foundation/path.hpp"
#include "timeline_event.hpp"

namespace scope::workspace
{

/**
 * @brief Receives projected timeline events from an artifact projector.
 */
class TimelineEventSink
{
  public:
    virtual ~TimelineEventSink() = default;

    /**
     * @return false when the sink is full and projection should stop for this artifact.
     */
    virtual bool append(TimelineEvent event) = 0;
};

/**
 * @brief Context passed to artifact projectors.
 */
struct ArtifactProjectionContext
{
    std::string investigationId;
    foundation::Path investigationRoot;
    TimelineProjectionOptions options;
};

/**
 * @brief Projects one artifact type into zero or more timeline events.
 */
class IArtifactProjector
{
  public:
    virtual ~IArtifactProjector() = default;

    [[nodiscard]] virtual std::string_view artifactType() const noexcept = 0;

    virtual void project(const ArtifactRecord& artifact, const foundation::Path& dataPath,
                         const ArtifactProjectionContext& context, TimelineEventSink& sink) const = 0;
};

/**
 * @brief Returns the projector for an artifact type string, or nullptr if unknown.
 */
[[nodiscard]] const IArtifactProjector* findArtifactProjector(std::string_view type) noexcept;

/**
 * @brief Builds a stable timeline event identifier.
 */
[[nodiscard]] std::string makeTimelineEventId(const std::string& investigationId, const std::string& artifactId,
                                              std::size_t sequenceWithinArtifact, const std::string& timestampIso,
                                              const std::string& eventType);

} // namespace scope::workspace
