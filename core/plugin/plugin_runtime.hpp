/**
 * @file plugin_runtime.hpp
 * @brief Helpers for CLI and integration tests to load plugins (M12).
 */

#pragma once

#include "extension_manager.hpp"
#include "plugin_config.hpp"
#include "plugin_load_stats.hpp"
#include "runtime/configuration.hpp"

namespace scope::plugin
{

[[nodiscard]] extension::ExtensionManager
createConfiguredExtensionManager(const runtime::Configuration& configuration,
                                 PluginLoadStats* statsOut = nullptr);

/**
 * @brief Destroys plugin-backed providers while their libraries are still loaded.
 */
void releasePluginRuntimeResources() noexcept;

/**
 * @brief Releases plugin providers and unloads libraries before process exit.
 */
void shutdownPluginRuntime() noexcept;

/**
 * @brief Returns false after shutdown begins so adapters skip cross-DLL destroy.
 */
[[nodiscard]] bool pluginProvidersMayDestroy() noexcept;

/**
 * @brief Registers an atexit handler to run @ref shutdownPluginRuntime once.
 */
void ensurePluginRuntimeShutdownRegistered() noexcept;

} // namespace scope::plugin
