/**
 * @file session_resource_cleanup.hpp
 * @brief Staged upload and tail cleanup for workspace sessions (M15.4).
 */

#pragma once

#include <string>

#include "session_store.hpp"

namespace scope::web
{

/** @brief Deletes tempUploadPath from disk and clears the field (caller may hold session mutex). */
void removeTempUploadFile(WorkspaceSession& session);

/** @brief Stops tail and removes staged upload temp files. */
void cleanupSessionResources(WorkspaceSession& session);

/** @brief Removes a superseded staged upload before assigning a new temp path (caller must hold session mutex). */
void replaceStagedUpload(WorkspaceSession& session, const std::string& newTempPath);

} // namespace scope::web
