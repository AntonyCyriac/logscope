/**
 * @file investigation_container.hpp
 * @brief Investigation aggregate root (Story 1 / v2.3.0).
 */

#pragma once

#include <vector>

#include "artifact_handler.hpp"
#include "artifact_record.hpp"
#include "foundation/path.hpp"
#include "foundation/result.hpp"

namespace scope::workspace
{

/**
 * @brief Input for creating a new investigation directory.
 */
struct InvestigationCreateRequest
{
    std::string name;
    std::string description;
};

/**
 * @brief Aggregate root for a portable investigation container.
 */
class Investigation
{
  public:
    [[nodiscard]] static foundation::Result<Investigation> create(const foundation::Path& investigationDir,
                                                                   InvestigationCreateRequest request);

    [[nodiscard]] static foundation::Result<Investigation> open(const foundation::Path& investigationDir);

    [[nodiscard]] const foundation::Path& rootDirectory() const noexcept;

    [[nodiscard]] const InvestigationManifest& manifest() const noexcept;

    [[nodiscard]] foundation::Result<ArtifactRecord> addArtifact(ArtifactIngestRequest request);

    [[nodiscard]] foundation::Result<bool> removeArtifact(const std::string& artifactId);

    [[nodiscard]] foundation::Result<ArtifactRecord> entryArtifact() const;

    [[nodiscard]] foundation::Result<foundation::Path> entryArtifactDataPath() const;

    [[nodiscard]] foundation::Result<foundation::Path> snapshotPath() const;

    [[nodiscard]] foundation::Result<bool> setEntryArtifact(const std::string& artifactId);

    [[nodiscard]] foundation::Result<bool> touchUpdatedAt();

    [[nodiscard]] foundation::Result<bool> updateSummary(const InvestigationSummary& summary);

    [[nodiscard]] foundation::Result<bool> persist();

  private:
    Investigation(foundation::Path investigationDir, InvestigationManifest manifest);

    [[nodiscard]] foundation::Result<bool> ensureArtifactUnderRoot(const foundation::Path& path) const;

    foundation::Path m_rootDirectory;
    InvestigationManifest m_manifest;
};

} // namespace scope::workspace
