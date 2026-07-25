/**
 * @file plugin_host_api.hpp
 * @brief Host-side plugin registration services (M12).
 */

#pragma once

#include <logscope/plugin/plugin.h>

#include <string>
#include <vector>

#include "extension_manager.hpp"

namespace scope::plugin
{

struct LoadedPluginRecord
{
    std::string id;
    std::string version;
    std::string description;
    std::string libraryPath;
    uint32_t apiVersion{0U};
};

/**
 * @brief Bridges C plugin ABI to LogScope registries and ExtensionManager.
 */
class PluginHostApi
{
  public:
    explicit PluginHostApi(extension::ExtensionManager& extensionManager);

    [[nodiscard]] LogScopeHostApi cApi() noexcept;

    void setCurrentLibraryPath(std::string path);

    [[nodiscard]] const std::vector<LoadedPluginRecord>& loadedPlugins() const noexcept;

  private:
    static int registerExtensionThunk(void* context, const LogScopePluginInfo* info);
    static int registerReportContributorThunk(void* context, const char* contributorId,
                                              LogScopeCreateReportContributorFn createFn);
    static int registerFormatParserThunk(void* context, const char* formatId,
                                         LogScopeCreateFormatParserFn createFn);
    static int registerSearchProviderThunk(void* context, const char* providerId,
                                           LogScopeCreateSearchProviderFn createFn);
    static int registerStorageBackendThunk(void* context, const char* backendId,
                                           LogScopeCreateStorageBackendFn createFn);

    extension::ExtensionManager& m_extensionManager;
    std::vector<LoadedPluginRecord> m_loadedPlugins;
    std::string m_currentLibraryPath;
    LogScopeHostApi m_cApi{};
};

} // namespace scope::plugin
