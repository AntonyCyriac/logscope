/**
 * @file plugin_registry.hpp
 * @brief Plugin registration.
 */

#pragma once

#include <string>
#include <vector>

namespace scope::runtime
{

/**
 * @brief Describes a registered plugin.
 */
struct PluginInfo
{
    std::string name;

    std::string version;
};

/**
 * @brief Maintains a registry of discovered plugins.
 */
class PluginRegistry
{
  public:
    /**
     * @brief Registers a plugin.
     *
     * @param info Plugin metadata.
     */
    void registerPlugin(PluginInfo info);

    /**
     * @brief Returns all registered plugins.
     */
    [[nodiscard]] const std::vector<PluginInfo>& plugins() const noexcept;

  private:
    std::vector<PluginInfo> m_plugins;
};

} // namespace scope::runtime
