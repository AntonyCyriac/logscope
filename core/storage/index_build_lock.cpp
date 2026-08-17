/**
 * @file index_build_lock.cpp
 */

#include "index_build_lock.hpp"

#include <chrono>
#include <filesystem>
#include <thread>

#include "sqlite_connection.hpp"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#endif

namespace scope::storage
{

namespace
{

constexpr const char* kIndexLockedMessage = "Index is locked by another process.";

[[nodiscard]] bool tryCreateExclusiveLockFile(const foundation::Path& lockPath)
{
    const std::filesystem::path path(lockPath.string());

#ifdef _WIN32
    const HANDLE handle =
        CreateFileW(path.wstring().c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (handle == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    CloseHandle(handle);
    return true;
#else
    const int descriptor =
        ::open(path.c_str(), O_CREAT | O_EXCL | O_WRONLY, static_cast<mode_t>(0644));

    if (descriptor < 0)
    {
        return false;
    }

    (void)::close(descriptor);
    return true;
#endif
}

} // namespace

IndexBuildLock::~IndexBuildLock()
{
    release();
}

IndexBuildLock::IndexBuildLock(IndexBuildLock&& other) noexcept
    : m_lockPath(std::move(other.m_lockPath)), m_held(other.m_held)
{
    other.m_held = false;
}

IndexBuildLock& IndexBuildLock::operator=(IndexBuildLock&& other) noexcept
{
    if (this != &other)
    {
        release();
        m_lockPath = std::move(other.m_lockPath);
        m_held = other.m_held;
        other.m_held = false;
    }

    return *this;
}

foundation::Result<IndexBuildLock> IndexBuildLock::acquire(const foundation::Path& databasePath)
{
    const foundation::Path lockPath = indexBuildLockPath(databasePath);
    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(kSqliteBusyTimeoutMs);

    while (true)
    {
        if (tryCreateExclusiveLockFile(lockPath))
        {
            IndexBuildLock lock;
            lock.m_lockPath = lockPath;
            lock.m_held = true;
            return foundation::Result<IndexBuildLock>(std::move(lock));
        }

        if (std::chrono::steady_clock::now() >= deadline)
        {
            return foundation::Result<IndexBuildLock>(
                foundation::Error(foundation::ErrorCode::IOError, kIndexLockedMessage));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void IndexBuildLock::release() noexcept
{
    if (!m_held)
    {
        return;
    }

    std::error_code error;
    std::filesystem::remove(m_lockPath.string(), error);

    m_held = false;
}

foundation::Path indexBuildingDatabasePath(const foundation::Path& databasePath)
{
    return foundation::Path(databasePath.string() + ".building");
}

foundation::Path indexBuildLockPath(const foundation::Path& databasePath)
{
    return foundation::Path(databasePath.string() + ".build.lock");
}

void removeDatabaseArtifacts(const foundation::Path& databasePath) noexcept
{
    std::error_code error;
    const std::filesystem::path base(databasePath.string());

    (void)std::filesystem::remove(base, error);
    (void)std::filesystem::remove(base.string() + "-wal", error);
    (void)std::filesystem::remove(base.string() + "-shm", error);
}

foundation::Result<bool> promoteBuildingDatabase(const foundation::Path& buildingPath,
                                               const foundation::Path& finalPath)
{
    std::error_code error;
    const std::filesystem::path building(buildingPath.string());
    const std::filesystem::path final(finalPath.string());

    if (!std::filesystem::exists(building, error))
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::IOError, "Index build database is missing."));
    }

    removeDatabaseArtifacts(finalPath);

    std::filesystem::rename(building, final, error);

    if (error)
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::IOError, error.message()));
    }

    const std::filesystem::path buildingWal = buildingPath.string() + "-wal";
    const std::filesystem::path finalWal = finalPath.string() + "-wal";
    const std::filesystem::path buildingShm = buildingPath.string() + "-shm";
    const std::filesystem::path finalShm = finalPath.string() + "-shm";

    if (std::filesystem::exists(buildingWal, error))
    {
        std::filesystem::rename(buildingWal, finalWal, error);

        if (error)
        {
            return foundation::Result<bool>(
                foundation::Error(foundation::ErrorCode::IOError, error.message()));
        }
    }

    if (std::filesystem::exists(buildingShm, error))
    {
        std::filesystem::rename(buildingShm, finalShm, error);

        if (error)
        {
            return foundation::Result<bool>(
                foundation::Error(foundation::ErrorCode::IOError, error.message()));
        }
    }

    return foundation::Result<bool>(true);
}

} // namespace scope::storage
