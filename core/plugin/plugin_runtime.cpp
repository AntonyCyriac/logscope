/**
 * @file plugin_runtime.cpp
 */

#include "plugin_runtime.hpp"

#include "plugin_loader.hpp"

namespace scope::plugin
{

extension::ExtensionManager createConfiguredExtensionManager(const runtime::Configuration& configuration)
{
    extension::ExtensionManager manager = extension::ExtensionManager::createWithBuiltIns();

    const PluginConfig pluginConfig = resolvePluginConfig(configuration);

    (void)loadPluginsForManager(manager, pluginConfig);

    manager.applyConfiguration(configuration);
    manager.initializeEnabled();

    return manager;
}

} // namespace scope::plugin
