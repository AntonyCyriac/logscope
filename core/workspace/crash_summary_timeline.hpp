/**
 * @file crash_summary_timeline.hpp
 * @brief Builds crash.summary timeline events from CrashReport (P1 / v2.9.0).
 */

#pragma once

#include "artifact_record.hpp"
#include "crash_report.hpp"
#include "foundation/result.hpp"
#include "timeline_event.hpp"

#include <optional>
#include <string>

namespace scope::workspace
{

/**
 * @brief Builds a crash.summary timeline event from an existing CrashReport.
 *
 * Returns nullopt when status is not_supported or importedAt is unusable.
 */
[[nodiscard]] std::optional<TimelineEvent> makeCrashSummaryTimelineEvent(
    const std::string& investigationId, const ArtifactRecord& artifact, const CrashReport& report);

} // namespace scope::workspace
