/**
 * @file cli_extension_runtime.hpp
 * @brief Shared extension manager bootstrap for CLI commands (M12).
 */

#pragma once

#include "extension_manager.hpp"
#include "plugin_runtime.hpp"
#include "runtime/configuration.hpp"

namespace scope::cli
{

[[nodiscard]] inline extension::ExtensionManager
createConfiguredExtensionManager(const runtime::Configuration& configuration)
{
    return plugin::createConfiguredExtensionManager(configuration);
}

} // namespace scope::cli
