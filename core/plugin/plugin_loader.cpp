/**
 * @file plugin_loader.cpp
 */

#include "plugin_loader.hpp"

#include "foundation/filesystem.hpp"
#include "foundation/stopwatch.hpp"
#include "log_macros.hpp"
#include "plugin_runtime.hpp"

namespace scope::plugin
{

namespace
{

void collectLibraryFiles(const foundation::Path& directory, std::vector<foundation::Path>& libraries)
{
    const auto filesResult = foundation::FileSystem::listRegularFiles(directory);

    if (!filesResult)
    {
        return;
    }

    for (const foundation::Path& entry : *filesResult)
    {
        if (isSharedLibraryFile(entry))
        {
            libraries.push_back(entry);
        }
    }
}

struct PersistentPluginLibraries
{
    std::vector<LoadedPluginLibrary> libraries;

    ~PersistentPluginLibraries()
    {
        releasePluginRuntimeResources();
        libraries.clear();
    }
};

PersistentPluginLibraries& persistentPluginState()
{
    static PersistentPluginLibraries state;

    return state;
}

std::vector<LoadedPluginLibrary>& persistentLoadedLibraries()
{
    return persistentPluginState().libraries;
}

} // namespace

PluginLoader::PluginLoader(extension::ExtensionManager& extensionManager)
    : m_extensionManager(extensionManager), m_hostApi(extensionManager)
{
}

foundation::Result<std::size_t> PluginLoader::loadFromPaths(const std::vector<foundation::Path>& searchPaths,
                                                            PluginLoadStats* statsOut)
{
    foundation::Stopwatch stopwatch;
    std::size_t loadedCount = 0U;
    std::size_t failedCount = 0U;
    std::size_t skippedPathCount = 0U;

    for (const foundation::Path& searchPath : searchPaths)
    {
        const auto existsResult = foundation::FileSystem::exists(searchPath);

        if (!existsResult || !*existsResult)
        {
            ++skippedPathCount;
            SCOPE_LOG_ERROR("plugin", "Plugin search path does not exist: " + searchPath.string());

            continue;
        }

        std::vector<foundation::Path> libraries;

        const auto directoryResult = foundation::FileSystem::isDirectory(searchPath);

        if (directoryResult && *directoryResult)
        {
            collectLibraryFiles(searchPath, libraries);
        }
        else if (isSharedLibraryFile(searchPath))
        {
            libraries.push_back(searchPath);
        }

        for (const foundation::Path& libraryPath : libraries)
        {
            const auto loadResult = loadLibraryFile(libraryPath);

            if (loadResult && *loadResult)
            {
                ++loadedCount;
            }
            else
            {
                ++failedCount;
            }
        }
    }

    if (statsOut != nullptr)
    {
        statsOut->attempted = true;
        statsOut->loaded = loadedCount;
        statsOut->failed = failedCount;
        statsOut->skippedPaths = skippedPathCount;
        statsOut->loadDuration = stopwatch.elapsed();
    }

    return foundation::Result<std::size_t>(loadedCount);
}

const std::vector<LoadedPluginLibrary>& PluginLoader::loadedLibraries() const noexcept
{
    return persistentLoadedLibraries();
}

foundation::Result<bool> PluginLoader::loadLibraryFile(const foundation::Path& libraryPath)
{
    auto libraryResult = SharedLibrary::load(libraryPath);

    if (!libraryResult)
    {
        SCOPE_LOG_ERROR("plugin", "Failed to load plugin library " + libraryPath.string() + ": " +
                                      libraryResult.error().message());

        return foundation::Result<bool>(false);
    }

    SharedLibrary library(std::move(libraryResult.value()));

    const auto symbolResult = library.resolveSymbol("logscope_plugin_register");

    if (!symbolResult)
    {
        SCOPE_LOG_ERROR("plugin", "Plugin missing entry symbol: " + libraryPath.string());

        return foundation::Result<bool>(false);
    }

    const auto registerFn = reinterpret_cast<LogScopePluginRegisterFn>(*symbolResult);

    m_hostApi.setCurrentLibraryPath(libraryPath.string());

    LogScopeHostApi hostApi = m_hostApi.cApi();
    const int registerStatus = registerFn(&hostApi);

    if (registerStatus != 0)
    {
        SCOPE_LOG_ERROR("plugin",
                        "Plugin registration failed for " + libraryPath.string() + " (code " +
                            std::to_string(registerStatus) + ")");

        return foundation::Result<bool>(false);
    }

    LoadedPluginLibrary loaded;
    loaded.path = libraryPath;
    loaded.library = std::move(library);

    if (!m_hostApi.loadedPlugins().empty())
    {
        loaded.pluginId = m_hostApi.loadedPlugins().back().id;
    }

    persistentLoadedLibraries().push_back(std::move(loaded));

    ensurePluginRuntimeShutdownRegistered();

    SCOPE_LOG_INFO("plugin", "Loaded plugin library: " + libraryPath.string());

    return foundation::Result<bool>(true);
}

foundation::Result<std::size_t> loadPluginsForManager(extension::ExtensionManager& manager,
                                                        const PluginConfig& config,
                                                        PluginLoadStats* statsOut)
{
    if (!config.enabled)
    {
        if (statsOut != nullptr)
        {
            *statsOut = PluginLoadStats{};
        }

        return foundation::Result<std::size_t>(0U);
    }

    PluginLoader loader(manager);

    return loader.loadFromPaths(mergePluginPaths(config), statsOut);
}

void unloadAllPersistentLibraries() noexcept
{
    releasePluginRuntimeResources();
    persistentLoadedLibraries().clear();
}

} // namespace scope::plugin
