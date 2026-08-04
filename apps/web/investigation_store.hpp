/**
 * @file investigation_store.hpp
 * @brief File-first investigation persistence (M15.5 / v2.3.0).
 */

#pragma once

#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "application_service.hpp"
#include "foundation/path.hpp"
#include "foundation/result.hpp"
#include "artifact_record.hpp"
#include "workspace.hpp"
#include "web_config.hpp"

namespace scope::web
{

/**
 * @brief Partial investigation metadata update from REST body.
 */
struct InvestigationUpdateRequest
{
    std::optional<std::string> name;
    std::optional<std::string> description;
    std::optional<std::string> primaryArtifactId;
};

/**
 * @brief List response payload.
 */
struct InvestigationListResult
{
    std::vector<scope::workspace::InvestigationManifest> investigations;
    bool truncated = false;
};

/**
 * @brief Persists investigations under web.workspace_dir.
 */
class InvestigationStore
{
  public:
    explicit InvestigationStore(const WebConfig& config);

    [[nodiscard]] const foundation::Path& rootDirectory() const noexcept;

    [[nodiscard]] foundation::Result<scope::workspace::InvestigationManifest> create(const std::string& name,
                                                                                     const std::string& description);

    [[nodiscard]] foundation::Result<InvestigationListResult> list(int limit) const;

    [[nodiscard]] foundation::Result<scope::workspace::InvestigationManifest> get(
        const std::string& investigationId) const;

    [[nodiscard]] foundation::Result<scope::workspace::InvestigationManifest> update(
        const std::string& investigationId, const InvestigationUpdateRequest& request);

    [[nodiscard]] foundation::Result<bool> remove(const std::string& investigationId);

    [[nodiscard]] foundation::Result<scope::workspace::ArtifactRecord> addLogArtifact(
        const std::string& investigationId, const foundation::Path& sourcePath, const std::string& displayName);

    [[nodiscard]] foundation::Result<scope::workspace::ArtifactRecord> addNoteArtifact(
        const std::string& investigationId, const std::string& title, const std::string& body);

    [[nodiscard]] foundation::Result<bool> removeArtifact(const std::string& investigationId,
                                                          const std::string& artifactId);

    [[nodiscard]] foundation::Result<foundation::Path> snapshotPathFor(const std::string& investigationId) const;

    [[nodiscard]] foundation::Result<foundation::Path> resolveSnapshotPath(const std::string& investigationId) const;

    [[nodiscard]] foundation::Result<foundation::Path> resolveEntryLogPath(const std::string& investigationId) const;

    void touchUpdatedAt(const std::string& investigationId);

    void updateSummaryFromService(const std::string& investigationId,
                                  const application::ApplicationService& service);

    [[nodiscard]] static bool isValidInvestigationId(const std::string& investigationId);

  private:
    [[nodiscard]] foundation::Path investigationDirectory(const std::string& investigationId) const;

    [[nodiscard]] foundation::Result<scope::workspace::Investigation> openInvestigation(
        const std::string& investigationId) const;

    [[nodiscard]] foundation::Result<bool> ensureUnderRoot(const foundation::Path& path) const;

    foundation::Path m_rootDirectory;
    mutable std::mutex m_mutex;
};

} // namespace scope::web
