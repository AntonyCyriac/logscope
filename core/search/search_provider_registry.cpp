/**
 * @file search_provider_registry.cpp
 */

#include "search_provider_registry.hpp"

namespace scope::search
{

SearchProviderRegistry& SearchProviderRegistry::instance()
{
    static SearchProviderRegistry registry;

    return registry;
}

void SearchProviderRegistry::registerProvider(const std::string& providerId, SearchProviderFactory factory)
{
    for (ProviderEntry& entry : m_providers)
    {
        if (entry.providerId == providerId)
        {
            entry.factory = std::move(factory);
            entry.cachedInstance.reset();

            return;
        }
    }

    ProviderEntry entry;
    entry.providerId = providerId;
    entry.factory = std::move(factory);

    m_providers.push_back(std::move(entry));
}

const SearchProvider* SearchProviderRegistry::findProvider(const std::string& providerId) const
{
    for (ProviderEntry& entry : m_providers)
    {
        if (entry.providerId != providerId)
        {
            continue;
        }

        if (entry.cachedInstance == nullptr && entry.factory)
        {
            entry.cachedInstance = entry.factory();
        }

        return entry.cachedInstance.get();
    }

    return nullptr;
}

std::vector<std::string> SearchProviderRegistry::registeredProviderIds() const
{
    std::vector<std::string> ids;
    ids.reserve(m_providers.size());

    for (const ProviderEntry& entry : m_providers)
    {
        ids.push_back(entry.providerId);
    }

    return ids;
}

void SearchProviderRegistry::clear()
{
    m_providers.clear();
}

} // namespace scope::search
