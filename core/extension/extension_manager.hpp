/**
 * @file extension_manager.hpp
 * @brief Discovers, configures, and manages supported extensions (C06).
 */

#pragma once

#include <string>
#include <vector>

#include "extension_descriptor.hpp"
#include "extension_info.hpp"
#include "foundation/result.hpp"
#include "runtime/configuration.hpp"

namespace scope::extension
{

/**
 * @brief Manages extension registration, configuration, and lifecycle.
 */
class ExtensionManager
{
  public:
    ExtensionManager();

    /**
     * @brief Creates a manager with all built-in extensions registered.
     */
    [[nodiscard]] static ExtensionManager createWithBuiltIns();

    /**
     * @brief Registers a built-in extension.
     */
    void registerBuiltIn(ExtensionDescriptor descriptor);

    /**
     * @brief Applies extension enablement from configuration.
     *
     * Keys use the form extensions.<id>.enabled=true|false.
     */
    void applyConfiguration(const runtime::Configuration& configuration);

    /**
     * @brief Initializes all enabled extensions without affecting unrelated capabilities.
     */
    void initializeEnabled();

    /**
     * @brief Returns metadata for all registered extensions.
     */
    [[nodiscard]] std::vector<ExtensionInfo> listExtensions() const;

    /**
     * @brief Returns metadata for a single extension.
     */
    [[nodiscard]] foundation::Result<ExtensionInfo> describe(const std::string& id) const;

  private:
    struct RegisteredExtension
    {
        ExtensionDescriptor descriptor;
        bool enabled = true;
        bool initialized = false;
        bool initializationFailed = false;
    };

    [[nodiscard]] const RegisteredExtension* findExtension(const std::string& id) const;

    [[nodiscard]] ExtensionInfo toInfo(const RegisteredExtension& extension) const noexcept;

    std::vector<RegisteredExtension> m_extensions;
};

void registerBuiltInExtensions(ExtensionManager& manager);

} // namespace scope::extension
