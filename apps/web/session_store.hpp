/**
 * @file session_store.hpp
 * @brief Server-side workspace sessions for logscope-web (M15.1).
 */

#pragma once

#include <chrono>
#include <functional>
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
    std::string boundInvestigationId;
    std::string activeArtifactId;
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

    void touchSession(const std::string& sessionId);

    [[nodiscard]] std::size_t evictIdleSessions(
        std::chrono::seconds idleTtl, const std::function<bool(const std::string&)>& skipSession = nullptr);

    [[nodiscard]] std::size_t evictSessionsForCapacity(
        std::size_t slotsNeeded, const std::function<bool(const std::string&)>& skipSession = nullptr);

    [[nodiscard]] bool removeSession(const std::string& sessionId);

    void cleanupAllSessionResources();

    [[nodiscard]] WorkspaceSession* findSession(const std::string& sessionId);

    [[nodiscard]] const WorkspaceSession* findSession(const std::string& sessionId) const;

    [[nodiscard]] std::size_t sessionCount() const;

  private:
    struct SessionEntry
    {
        WorkspaceSession workspace;
        std::chrono::steady_clock::time_point lastActivityAt{std::chrono::steady_clock::now()};
    };

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, SessionEntry> m_sessions;
};

} // namespace scope::web
