/**
 * @file evidence_link.hpp
 * @brief Investigation evidence link types (Story 5 / v2.7.0).
 */

#pragma once

#include <optional>
#include <string>

namespace scope::workspace
{

/**
 * @brief Directed relationship type between timeline events.
 */
enum class EvidenceLinkType
{
    Precedes,
    Follows,
    Supports,
    Related
};

/**
 * @brief Endpoint anchor for an evidence link.
 */
struct LinkEndpoint
{
    std::string kind;    // "timeline_event" in P0.1
    std::string eventId;
};

/**
 * @brief Persisted investigator-authored link between evidence nodes.
 */
struct EvidenceLink
{
    std::string id;
    EvidenceLinkType type;
    LinkEndpoint source;
    LinkEndpoint target;
    std::string createdAt;
    std::optional<std::string> note;
};

/**
 * @brief Computed link status relative to current timeline projection.
 */
enum class EvidenceLinkStatus
{
    Active,
    Stale
};

/**
 * @brief Evidence link with API-computed status (not persisted).
 */
struct EvidenceLinkRecord : EvidenceLink
{
    EvidenceLinkStatus status = EvidenceLinkStatus::Active;
};

/**
 * @brief Input for creating a new evidence link.
 */
struct EvidenceLinkCreateRequest
{
    EvidenceLinkType type;
    LinkEndpoint source;
    LinkEndpoint target;
    std::optional<std::string> note;
};

[[nodiscard]] std::string evidenceLinkTypeToString(EvidenceLinkType type);

[[nodiscard]] std::optional<EvidenceLinkType> parseEvidenceLinkType(const std::string& value);

[[nodiscard]] std::string evidenceLinkStatusToString(EvidenceLinkStatus status);

} // namespace scope::workspace
