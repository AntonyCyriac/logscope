/**
 * @file workspace_store.hpp
 * @brief File-first shared workspace persistence (M15.3 / C12f).
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
#include "investigation_store.hpp"
#include "web_config.hpp"

namespace scope::web
{

/**
 * @brief Reference to the workspace source file.
 */
struct WorkspaceSourceRef
{
    std::string type = "upload";
    std::string displayName;
    std::string path;
};

/**
 * @brief Summary counts stored in workspace metadata.
 */
struct WorkspaceSummary
{
    bool hasModel = false;
    std::uint64_t lineCount = 0;
    std::uint64_t errorCount = 0;
};

/**
 * @brief Shared workspace metadata (no investigation rows).
 */
struct WorkspaceMetadata
{
    std::string id;
    std::string name;
    std::string description;
    std::string createdAt;
    std::string updatedAt;
    WorkspaceSourceRef sourceRef;
    WorkspaceSummary summary;
    std::string snapshotFile = "snapshot.session";
};

/**
 * @brief Create-workspace input from REST body.
 */
struct WorkspaceCreateRequest
{
    std::string name;
    std::string description;
    std::optional<WorkspaceSourceRef> sourceRef;
    bool captureSession = false;
};

/**
 * @brief Partial metadata update from REST body.
 */
struct WorkspaceUpdateRequest
{
    std::optional<std::string> name;
    std::optional<std::string> description;
    std::optional<WorkspaceSourceRef> sourceRef;
};

/**
 * @brief List response payload.
 */
struct WorkspaceListResult
{
    std::vector<WorkspaceMetadata> workspaces;
    bool truncated = false;
};

/**
 * @brief Persists shared workspaces under web.workspace_dir.
 */
class WorkspaceStore
{
  public:
    explicit WorkspaceStore(const WebConfig& config);

    [[nodiscard]] const foundation::Path& rootDirectory() const noexcept;

    [[nodiscard]] foundation::Result<WorkspaceMetadata> create(const WorkspaceCreateRequest& request);

    [[nodiscard]] foundation::Result<WorkspaceListResult> list(int limit) const;

    [[nodiscard]] foundation::Result<WorkspaceMetadata> getMetadata(const std::string& workspaceId) const;

    [[nodiscard]] foundation::Result<WorkspaceMetadata> updateMetadata(const std::string& workspaceId,
                                                                         const WorkspaceUpdateRequest& request);

    [[nodiscard]] foundation::Result<bool> remove(const std::string& workspaceId);

    [[nodiscard]] foundation::Result<foundation::Path> resolveSnapshotPath(const std::string& workspaceId) const;

    [[nodiscard]] foundation::Result<foundation::Path> snapshotPathFor(const std::string& workspaceId) const;

    void touchUpdatedAt(const std::string& workspaceId);

    void updateSummaryFromService(const std::string& workspaceId, const application::ApplicationService& service);

    [[nodiscard]] static bool isValidWorkspaceId(const std::string& workspaceId);

    [[nodiscard]] InvestigationStore& investigationStore() noexcept;

    [[nodiscard]] const InvestigationStore& investigationStore() const noexcept;

  private:
    InvestigationStore m_investigationStore;
};

} // namespace scope::web
