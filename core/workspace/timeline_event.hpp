/**
 * @file timeline_event.hpp
 * @brief Investigation timeline projection types (Story 3 / v2.5.0).
 */

#pragma once

#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace scope::workspace
{

/**
 * @brief Provenance for navigating from a timeline event back to an artifact.
 */
struct EventSource
{
    std::string artifactId;
    std::string artifactType;
    std::string artifactName;
    std::optional<std::size_t> lineNumber;
    std::optional<std::size_t> byteOffset;
};

/**
 * @brief A single event in the chronological investigation narrative.
 */
struct TimelineEvent
{
    std::string id;
    std::string timestamp;
    std::string artifactId;
    std::string eventType;
    std::optional<std::string> severity;
    std::string message;
    EventSource source;
    std::map<std::string, std::string> metadata;
};

/**
 * @brief Sort direction for timeline projection.
 */
enum class TimelineSortOrder
{
    Ascending,
    Descending
};

/**
 * @brief Options controlling timeline projection (pagination and caps).
 */
struct TimelineProjectionOptions
{
    std::size_t limit = 10'000U;
    std::size_t offset = 0U;
    std::size_t maxEventsPerArtifact = 5'000U;
    TimelineSortOrder order = TimelineSortOrder::Ascending;
};

/**
 * @brief Result of projecting a timeline from investigation artifacts.
 */
struct TimelineProjectionResult
{
    std::vector<TimelineEvent> events;
    bool truncated = false;
    std::optional<std::size_t> totalMatched;
    std::vector<std::string> warnings;
};

} // namespace scope::workspace
