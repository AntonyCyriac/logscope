/**
 * @file plugin_registry.cpp
 * @brief PluginRegistry implementation.
 */

#include "plugin_registry.hpp"

namespace scope::runtime
{

void PluginRegistry::registerPlugin(PluginInfo info)
{
    m_plugins.push_back(std::move(info));
}

const std::vector<PluginInfo>& PluginRegistry::plugins() const noexcept
{
    return m_plugins;
}

} // namespace scope::runtime
