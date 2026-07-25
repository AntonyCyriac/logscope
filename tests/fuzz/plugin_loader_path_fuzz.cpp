/**
 * @file plugin_loader_path_fuzz.cpp
 * @brief libFuzzer smoke target for malformed plugin paths (M12).
 */

#include <cstddef>
#include <cstdint>
#include <string>

#include "extension_manager.hpp"
#include "foundation/path.hpp"
#include "plugin_config.hpp"
#include "plugin_loader.hpp"

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, const std::size_t size)
{
    if (size == 0U)
    {
        return 0;
    }

    const std::string pathValue(reinterpret_cast<const char*>(data), size);
    scope::extension::ExtensionManager manager = scope::extension::ExtensionManager::createWithBuiltIns();

    scope::plugin::PluginConfig config;
    config.enabled = true;
    config.paths = {scope::foundation::Path(pathValue)};

    (void)scope::plugin::loadPluginsForManager(manager, config);

    return 0;
}
