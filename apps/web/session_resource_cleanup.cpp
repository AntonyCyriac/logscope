/**
 * @file session_resource_cleanup.cpp
 */

#include "session_resource_cleanup.hpp"

#include <filesystem>
#include <mutex>

namespace scope::web
{

namespace
{

void deletePathIfPresent(const std::string& path)
{
    if (path.empty())
    {
        return;
    }

    std::error_code errorCode;
    std::filesystem::remove(path, errorCode);
}

} // namespace

void removeTempUploadFile(WorkspaceSession& session)
{
    deletePathIfPresent(session.tempUploadPath);
    session.tempUploadPath.clear();
}

void cleanupSessionResources(WorkspaceSession& session)
{
    std::lock_guard<std::mutex> lock(session.mutex);

    if (session.service != nullptr && session.service->isTailing())
    {
        session.service->stopTail();
    }

    removeTempUploadFile(session);
}

void replaceStagedUpload(WorkspaceSession& session, const std::string& newTempPath)
{
    std::lock_guard<std::mutex> lock(session.mutex);

    if (!session.tempUploadPath.empty() && session.tempUploadPath != newTempPath)
    {
        deletePathIfPresent(session.tempUploadPath);
    }

    session.tempUploadPath = newTempPath;
}

} // namespace scope::web
