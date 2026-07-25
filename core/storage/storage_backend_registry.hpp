/**
 * @file storage_backend_registry.hpp
 * @brief Registry for plugin IndexStore backends (M12).
 */

#pragma once

#include <functional>
#include <string>
#include <vector>

#include "foundation/path.hpp"
#include "foundation/result.hpp"
#include "index_fingerprint.hpp"
#include "index_store.hpp"
#include "log_format.hpp"
#include "storage_config.hpp"

namespace scope::storage
{

using StorageBackendFactory =
    std::function<foundation::Result<IndexStorePtr>(const StorageConfig& config,
                                                      const IndexFingerprint& fingerprint,
                                                      const foundation::Path& sourcePath,
                                                      analysis::LogFormat format)>;

class StorageBackendRegistry
{
  public:
    [[nodiscard]] static StorageBackendRegistry& instance();

    void registerBackend(const std::string& backendId, StorageBackendFactory factory);

    [[nodiscard]] StorageBackendFactory findFactory(const std::string& backendId) const;

    [[nodiscard]] std::vector<std::string> registeredBackendIds() const;

    void clear();

  private:
    StorageBackendRegistry() = default;

    struct BackendEntry
    {
        std::string backendId;
        StorageBackendFactory factory;
    };

    std::vector<BackendEntry> m_backends;
};

} // namespace scope::storage
