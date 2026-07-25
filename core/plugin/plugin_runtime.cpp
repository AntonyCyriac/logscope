/**
 * @file plugin_runtime.cpp
 */

#include "plugin_runtime.hpp"

#include <atomic>
#include <cstdlib>
#include <mutex>

#include "parser_registry.hpp"
#include "plugin_loader.hpp"
#include "report_section_renderer.hpp"
#include "search_provider_registry.hpp"
#include "storage_backend_registry.hpp"

namespace scope::plugin
{

namespace
{

std::atomic<bool> g_pluginProvidersMayDestroy{true};

} // namespace

extension::ExtensionManager createConfiguredExtensionManager(const runtime::Configuration& configuration)
{
    extension::ExtensionManager manager = extension::ExtensionManager::createWithBuiltIns();

    const PluginConfig pluginConfig = resolvePluginConfig(configuration);

    (void)loadPluginsForManager(manager, pluginConfig);

    manager.applyConfiguration(configuration);
    manager.initializeEnabled();

    return manager;
}

bool pluginProvidersMayDestroy() noexcept
{
    return g_pluginProvidersMayDestroy.load();
}

void releasePluginRuntimeResources() noexcept
{
    static bool resourcesReleased = false;

    if (resourcesReleased)
    {
        return;
    }

    resourcesReleased = true;

    analysis::ParserRegistry::instance().clear();
    search::SearchProviderRegistry::instance().clear();
    storage::StorageBackendRegistry::instance().clear();
    reporting::ReportSectionRegistry::instance().clearPluginContributors();

    g_pluginProvidersMayDestroy.store(false);
}

void shutdownPluginRuntime() noexcept
{
    releasePluginRuntimeResources();
    unloadAllPersistentLibraries();
}

void ensurePluginRuntimeShutdownRegistered() noexcept
{
    static std::once_flag registered;

    std::call_once(registered, []() { std::atexit(&shutdownPluginRuntime); });
}

} // namespace scope::plugin
