/**
 * @file storage_backend_registry.cpp
 */

#include "storage_backend_registry.hpp"

namespace scope::storage
{

StorageBackendRegistry& StorageBackendRegistry::instance()
{
    static StorageBackendRegistry registry;

    return registry;
}

void StorageBackendRegistry::registerBackend(const std::string& backendId, StorageBackendFactory factory)
{
    for (BackendEntry& entry : m_backends)
    {
        if (entry.backendId == backendId)
        {
            entry.factory = std::move(factory);

            return;
        }
    }

    BackendEntry entry;
    entry.backendId = backendId;
    entry.factory = std::move(factory);

    m_backends.push_back(std::move(entry));
}

StorageBackendFactory StorageBackendRegistry::findFactory(const std::string& backendId) const
{
    for (const BackendEntry& entry : m_backends)
    {
        if (entry.backendId == backendId)
        {
            return entry.factory;
        }
    }

    return nullptr;
}

std::vector<std::string> StorageBackendRegistry::registeredBackendIds() const
{
    std::vector<std::string> ids;
    ids.reserve(m_backends.size());

    for (const BackendEntry& entry : m_backends)
    {
        ids.push_back(entry.backendId);
    }

    return ids;
}

void StorageBackendRegistry::clear()
{
    m_backends.clear();
}

} // namespace scope::storage
