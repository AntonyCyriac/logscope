/**
 * @file timeline_projector.hpp
 * @brief Merges artifact projections into a paginated timeline (Story 3 / v2.5.0).
 */

#pragma once

#include "artifact_record.hpp"
#include "foundation/path.hpp"
#include "foundation/result.hpp"
#include "timeline_event.hpp"

namespace scope::workspace
{

/**
 * @brief Projects a chronological timeline from an investigation manifest.
 */
class TimelineProjector
{
  public:
    [[nodiscard]] static foundation::Result<TimelineProjectionResult> project(
        const foundation::Path& investigationRoot, const InvestigationManifest& manifest,
        TimelineProjectionOptions options);
};

} // namespace scope::workspace
