/**
 * @file session_store.hpp
 * @brief Server-side workspace sessions for logscope-web (M15.1).
 */

#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

#include "application_service.hpp"

namespace scope::web
{

/**
 * @brief A single workspace with its own ApplicationService instance.
 */
struct WorkspaceSession
{
    std::unique_ptr<application::ApplicationService> service;
    mutable std::mutex mutex;
    std::string tempUploadPath;
};

/**
 * @brief Maps session UUIDs to workspace state.
 */
class SessionStore
{
  public:
    [[nodiscard]] std::string createWorkspace();

    /**
     * @brief Returns an existing session or creates one when id is empty.
     */
    [[nodiscard]] std::string resolveSession(const std::string& sessionId, bool autoCreate);

    [[nodiscard]] WorkspaceSession* findSession(const std::string& sessionId);

    [[nodiscard]] const WorkspaceSession* findSession(const std::string& sessionId) const;

    [[nodiscard]] std::size_t sessionCount() const;

  private:
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, WorkspaceSession> m_sessions;
};

} // namespace scope::web
