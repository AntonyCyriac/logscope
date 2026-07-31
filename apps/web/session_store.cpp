/**
 * @file session_store.cpp
 */

#include "session_store.hpp"

#include "session_resource_cleanup.hpp"

#include "foundation/uuid.hpp"

#include <algorithm>
#include <vector>

namespace scope::web
{

std::string SessionStore::createWorkspace()
{
    const std::string sessionId = foundation::Uuid::generate().toString();

    std::lock_guard<std::mutex> lock(m_mutex);
    SessionEntry& entry = m_sessions[sessionId];
    entry.workspace.service = std::make_unique<application::ApplicationService>();
    entry.lastActivityAt = std::chrono::steady_clock::now();

    return sessionId;
}

std::string SessionStore::resolveSession(const std::string& sessionId, const bool autoCreate)
{
    if (!sessionId.empty())
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        const auto iterator = m_sessions.find(sessionId);

        if (iterator != m_sessions.end())
        {
            iterator->second.lastActivityAt = std::chrono::steady_clock::now();

            return sessionId;
        }

        // Stale or unknown session id — never silently allocate a new workspace.
        return {};
    }

    if (!autoCreate)
    {
        return {};
    }

    return createWorkspace();
}

void SessionStore::touchSession(const std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    const auto iterator = m_sessions.find(sessionId);

    if (iterator != m_sessions.end())
    {
        iterator->second.lastActivityAt = std::chrono::steady_clock::now();
    }
}

std::size_t SessionStore::evictIdleSessions(const std::chrono::seconds idleTtl,
                                            const std::function<bool(const std::string&)>& skipSession)
{
    if (idleTtl.count() <= 0)
    {
        return 0U;
    }

    const auto now = std::chrono::steady_clock::now();
    std::vector<std::string> candidates;

    {
        std::lock_guard<std::mutex> lock(m_mutex);

        for (const auto& entry : m_sessions)
        {
            if ((now - entry.second.lastActivityAt) < idleTtl)
            {
                continue;
            }

            if (skipSession != nullptr && skipSession(entry.first))
            {
                continue;
            }

            candidates.push_back(entry.first);
        }
    }

    std::size_t evicted = 0U;

    for (const std::string& sessionId : candidates)
    {
        if (removeSession(sessionId))
        {
            ++evicted;
        }
    }

    return evicted;
}

std::size_t SessionStore::evictSessionsForCapacity(const std::size_t slotsNeeded,
                                                   const std::function<bool(const std::string&)>& skipSession)
{
    if (slotsNeeded == 0U)
    {
        return 0U;
    }

    std::vector<std::pair<std::string, std::chrono::steady_clock::time_point>> ordered;

    {
        std::lock_guard<std::mutex> lock(m_mutex);

        for (const auto& entry : m_sessions)
        {
            if (skipSession != nullptr && skipSession(entry.first))
            {
                continue;
            }

            ordered.emplace_back(entry.first, entry.second.lastActivityAt);
        }
    }

    std::sort(ordered.begin(), ordered.end(),
              [](const auto& left, const auto& right) { return left.second < right.second; });

    std::size_t evicted = 0U;

    for (const auto& candidate : ordered)
    {
        if (evicted >= slotsNeeded)
        {
            break;
        }

        if (removeSession(candidate.first))
        {
            ++evicted;
        }
    }

    return evicted;
}

bool SessionStore::removeSession(const std::string& sessionId)
{
    WorkspaceSession workspaceToCleanup;

    {
        std::lock_guard<std::mutex> lock(m_mutex);

        const auto iterator = m_sessions.find(sessionId);

        if (iterator == m_sessions.end())
        {
            return false;
        }

        workspaceToCleanup.service = std::move(iterator->second.workspace.service);
        workspaceToCleanup.tempUploadPath = iterator->second.workspace.tempUploadPath;
        m_sessions.erase(iterator);
    }

    cleanupSessionResources(workspaceToCleanup);

    return true;
}

void SessionStore::cleanupAllSessionResources()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    for (auto& entry : m_sessions)
    {
        cleanupSessionResources(entry.second.workspace);
    }
}

WorkspaceSession* SessionStore::findSession(const std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    const auto iterator = m_sessions.find(sessionId);

    if (iterator == m_sessions.end())
    {
        return nullptr;
    }

    return &iterator->second.workspace;
}

const WorkspaceSession* SessionStore::findSession(const std::string& sessionId) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    const auto iterator = m_sessions.find(sessionId);

    if (iterator == m_sessions.end())
    {
        return nullptr;
    }

    return &iterator->second.workspace;
}

std::size_t SessionStore::sessionCount() const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_sessions.size();
}

} // namespace scope::web
