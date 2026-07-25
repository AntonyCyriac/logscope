/**
 * @file plugin_config.hpp
 * @brief Plugin loading configuration (M12).
 */

#pragma once

#include <string>
#include <vector>

#include "foundation/path.hpp"
#include "foundation/result.hpp"
#include "runtime/configuration.hpp"

namespace scope::plugin
{

struct PluginConfig
{
    bool enabled{false};
    std::vector<foundation::Path> paths;
};

[[nodiscard]] PluginConfig resolvePluginConfig(const runtime::Configuration& configuration) noexcept;

[[nodiscard]] std::vector<foundation::Path> mergePluginPaths(const PluginConfig& config);

[[nodiscard]] foundation::Result<bool> validatePluginConfiguration(
    const runtime::Configuration& configuration) noexcept;

} // namespace scope::plugin
