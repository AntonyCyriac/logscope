/**
 * @file plugin_runtime.hpp
 * @brief Helpers for CLI and integration tests to load plugins (M12).
 */

#pragma once

#include "extension_manager.hpp"
#include "plugin_config.hpp"
#include "runtime/configuration.hpp"

namespace scope::plugin
{

[[nodiscard]] extension::ExtensionManager
createConfiguredExtensionManager(const runtime::Configuration& configuration);

} // namespace scope::plugin
