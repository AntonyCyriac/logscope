/**
 * @file session_store.cpp
 */

#include "session_store.hpp"

#include "foundation/uuid.hpp"

namespace scope::web
{

std::string SessionStore::createWorkspace()
{
    const std::string sessionId = foundation::Uuid::generate().toString();

    std::lock_guard<std::mutex> lock(m_mutex);
    WorkspaceSession& workspace = m_sessions[sessionId];
    workspace.service = std::make_unique<application::ApplicationService>();

    return sessionId;
}

std::string SessionStore::resolveSession(const std::string& sessionId, const bool autoCreate)
{
    if (!sessionId.empty())
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_sessions.find(sessionId) != m_sessions.end())
        {
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

WorkspaceSession* SessionStore::findSession(const std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    const auto iterator = m_sessions.find(sessionId);

    if (iterator == m_sessions.end())
    {
        return nullptr;
    }

    return &iterator->second;
}

const WorkspaceSession* SessionStore::findSession(const std::string& sessionId) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    const auto iterator = m_sessions.find(sessionId);

    if (iterator == m_sessions.end())
    {
        return nullptr;
    }

    return &iterator->second;
}

std::size_t SessionStore::sessionCount() const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_sessions.size();
}

} // namespace scope::web
