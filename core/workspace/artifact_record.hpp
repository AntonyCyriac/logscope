/**
 * @file artifact_record.hpp
 * @brief Investigation artifact metadata (Story 1 / v2.3.0).
 */

#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "evidence_link.hpp"

namespace scope::workspace
{

/**
 * @brief Provenance for an imported artifact.
 */
struct ArtifactSource
{
    std::string kind = "upload";
    std::string displayName;
};

/**
 * @brief Artifact entity within an investigation aggregate.
 */
struct ArtifactRecord
{
    std::string id;
    std::string type;
    std::string name;
    std::string relativePath;
    std::string importedAt;
    /// Source file last-write time at import (ISO-8601). Empty when unknown or not file-backed.
    std::string sourceModifiedAt;
    ArtifactSource source;
    std::string status = "ready";
    std::vector<std::string> tags;
    std::map<std::string, std::string> metadata;
};

/**
 * @brief Cached analyze summary on the investigation manifest.
 */
struct InvestigationSummary
{
    bool hasModel = false;
    std::uint64_t lineCount = 0U;
    std::uint64_t errorCount = 0U;
};

/**
 * @brief On-disk investigation manifest (schema version 1 or 2).
 */
struct InvestigationManifest
{
    int schemaVersion = 1;
    std::string id;
    std::string name;
    std::string description;
    std::string createdAt;
    std::string updatedAt;
    std::string primaryArtifactId;
    std::vector<std::string> tags;
    std::map<std::string, std::string> metadata;
    std::vector<ArtifactRecord> artifacts;
    InvestigationSummary summary;
    std::string snapshotFile = "snapshot.session";
    std::vector<EvidenceLink> evidenceLinks;
};

} // namespace scope::workspace
