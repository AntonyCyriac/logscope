/**
 * @file investigation_manifest_io.hpp
 * @brief Read/write investigation manifest.json.
 */

#pragma once

#include "artifact_record.hpp"
#include "foundation/path.hpp"
#include "foundation/result.hpp"

namespace scope::workspace
{

[[nodiscard]] foundation::Result<InvestigationManifest> loadManifest(const foundation::Path& investigationDir);

[[nodiscard]] foundation::Result<bool> saveManifest(const foundation::Path& investigationDir,
                                                    const InvestigationManifest& manifest);

/**
 * @brief Builds manifest v1 from legacy workspace.json metadata.
 */
[[nodiscard]] foundation::Result<InvestigationManifest> migrateLegacyWorkspaceMetadata(
    const foundation::Path& investigationDir, const std::string& workspaceJson);

} // namespace scope::workspace
