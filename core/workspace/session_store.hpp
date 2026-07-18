/**
 * @file session_store.hpp
 * @brief Persists investigation sessions to disk.
 */

#pragma once

#include <vector>

#include "foundation/path.hpp"
#include "foundation/result.hpp"
#include "investigation_session.hpp"

namespace scope::workspace
{

/**
 * @brief Saves, loads, and lists investigation session files.
 */
class SessionStore
{
  public:
    /**
     * @brief Saves a session to a .logscope-session file.
     */
    [[nodiscard]] foundation::Result<bool> save(const InvestigationSession& session,
                                                const foundation::Path& path) const;

    /**
     * @brief Loads a session from disk.
     */
    [[nodiscard]] foundation::Result<InvestigationSession> load(const foundation::Path& path) const;

    /**
     * @brief Lists session files in a directory.
     */
    [[nodiscard]] foundation::Result<std::vector<foundation::Path>> list(const foundation::Path& directory) const;
};

} // namespace scope::workspace
