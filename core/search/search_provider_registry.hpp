/**
 * @file search_provider_registry.hpp
 * @brief Registry for plugin search providers (M12).
 */

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "search_provider.hpp"

namespace scope::search
{

using SearchProviderFactory = std::function<std::unique_ptr<SearchProvider>()>;

class SearchProviderRegistry
{
  public:
    [[nodiscard]] static SearchProviderRegistry& instance();

    void registerProvider(const std::string& providerId, SearchProviderFactory factory);

    [[nodiscard]] const SearchProvider* findProvider(const std::string& providerId) const;

    [[nodiscard]] std::vector<std::string> registeredProviderIds() const;

    void clear();

  private:
    SearchProviderRegistry() = default;

    struct ProviderEntry
    {
        std::string providerId;
        SearchProviderFactory factory;
        std::unique_ptr<SearchProvider> cachedInstance;
    };

    mutable std::vector<ProviderEntry> m_providers;
};

} // namespace scope::search
