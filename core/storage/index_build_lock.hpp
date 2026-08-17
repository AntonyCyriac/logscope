/**
 * @file index_build_lock.hpp
 * @brief Exclusive lock for concurrent-safe SQLite index creation.
 */

#pragma once

#include "foundation/path.hpp"
#include "foundation/result.hpp"

namespace scope::storage
{

class IndexBuildLock
{
  public:
    IndexBuildLock() = default;
    ~IndexBuildLock();

    IndexBuildLock(IndexBuildLock&& other) noexcept;
    IndexBuildLock& operator=(IndexBuildLock&& other) noexcept;

    IndexBuildLock(const IndexBuildLock&) = delete;
    IndexBuildLock& operator=(const IndexBuildLock&) = delete;

    [[nodiscard]] static foundation::Result<IndexBuildLock> acquire(const foundation::Path& databasePath);

    void release() noexcept;

    [[nodiscard]] bool isHeld() const noexcept { return m_held; }

  private:
    foundation::Path m_lockPath;
    bool m_held{false};
};

[[nodiscard]] foundation::Path indexBuildingDatabasePath(const foundation::Path& databasePath);

[[nodiscard]] foundation::Path indexBuildLockPath(const foundation::Path& databasePath);

void removeDatabaseArtifacts(const foundation::Path& databasePath) noexcept;

[[nodiscard]] foundation::Result<bool> promoteBuildingDatabase(const foundation::Path& buildingPath,
                                                               const foundation::Path& finalPath);

} // namespace scope::storage
