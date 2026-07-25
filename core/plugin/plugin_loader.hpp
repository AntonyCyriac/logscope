/**
 * @file plugin_loader.hpp
 * @brief Discovers and loads LogScope plugins from shared libraries.
 */

#pragma once

#include <vector>

#include "extension_manager.hpp"
#include "foundation/path.hpp"
#include "foundation/result.hpp"
#include "plugin_config.hpp"
#include "plugin_host_api.hpp"
#include "shared_library.hpp"

namespace scope::plugin
{

struct LoadedPluginLibrary
{
    foundation::Path path;
    SharedLibrary library;
    std::string pluginId;
};

/**
 * @brief Loads plugins from directories and registers providers with the host API.
 */
class PluginLoader
{
  public:
    explicit PluginLoader(extension::ExtensionManager& extensionManager);

    [[nodiscard]] foundation::Result<std::size_t> loadFromPaths(const std::vector<foundation::Path>& searchPaths);

    [[nodiscard]] const std::vector<LoadedPluginLibrary>& loadedLibraries() const noexcept;

  private:
    [[nodiscard]] foundation::Result<bool> loadLibraryFile(const foundation::Path& libraryPath);

    extension::ExtensionManager& m_extensionManager;
    PluginHostApi m_hostApi;
};

[[nodiscard]] foundation::Result<std::size_t>
loadPluginsForManager(extension::ExtensionManager& manager, const PluginConfig& config);

void unloadAllPersistentLibraries() noexcept;

} // namespace scope::plugin
