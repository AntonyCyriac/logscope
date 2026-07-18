/**
 * @file session_store.cpp
 * @brief SessionStore implementation.
 */

#include "session_store.hpp"

#include <fstream>
#include <sstream>

#include "foundation/error.hpp"
#include "foundation/filesystem.hpp"
#include "foundation/string.hpp"
#include "foundation/string.hpp"
#include "log_macros.hpp"
#include "session_serializer.hpp"

namespace scope::workspace
{

namespace
{

constexpr std::string_view sessionSuffix = ".logscope-session";

bool hasSessionSuffix(const foundation::Path& path)
{
    return foundation::endsWith(path.string(), sessionSuffix);
}

} // namespace

foundation::Result<bool> SessionStore::save(const InvestigationSession& session,
                                           const foundation::Path& path) const
{
    SCOPE_LOG_INFO("workspace", "Saving session to " + path.string());

    std::ofstream stream(path.string());

    if (!stream)
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::IOError, "Unable to write session file."));
    }

    stream << SessionSerializer::serialize(session);

    if (!stream)
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::IOError, "Failed to write session file."));
    }

    return foundation::Result<bool>(true);
}

foundation::Result<InvestigationSession> SessionStore::load(const foundation::Path& path) const
{
    SCOPE_LOG_INFO("workspace", "Loading session from " + path.string());

    const auto existsResult = foundation::FileSystem::exists(path);

    if (!existsResult)
    {
        return foundation::Result<InvestigationSession>(existsResult.error());
    }

    if (!*existsResult)
    {
        return foundation::Result<InvestigationSession>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Session file not found."));
    }

    std::ifstream stream(path.string());

    if (!stream)
    {
        return foundation::Result<InvestigationSession>(
            foundation::Error(foundation::ErrorCode::IOError, "Unable to open session file."));
    }

    std::ostringstream content;
    content << stream.rdbuf();

    return SessionSerializer::deserialize(content.str());
}

foundation::Result<std::vector<foundation::Path>> SessionStore::list(const foundation::Path& directory) const
{
    const auto filesResult = foundation::FileSystem::listRegularFiles(directory);

    if (!filesResult)
    {
        return foundation::Result<std::vector<foundation::Path>>(filesResult.error());
    }

    std::vector<foundation::Path> sessionFiles;

    for (const foundation::Path& file : *filesResult)
    {
        if (hasSessionSuffix(file))
        {
            sessionFiles.push_back(file);
        }
    }

    return foundation::Result<std::vector<foundation::Path>>(std::move(sessionFiles));
}

} // namespace scope::workspace
