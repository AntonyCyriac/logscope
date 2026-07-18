/**
 * @file extension_manager.cpp
 * @brief ExtensionManager implementation.
 */

#include "extension_manager.hpp"

#include "foundation/error.hpp"
#include "foundation/string.hpp"
#include "log_macros.hpp"

namespace scope::extension
{

namespace
{

std::string configurationKeyForExtension(const std::string& id)
{
    return "extensions." + id + ".enabled";
}

bool parseEnabledValue(const std::string& value)
{
    const std::string normalized = foundation::toLower(foundation::trim(value));

    return normalized == "true" || normalized == "1" || normalized == "yes" || normalized == "on";
}

} // namespace

ExtensionManager::ExtensionManager() = default;

ExtensionManager ExtensionManager::createWithBuiltIns()
{
    ExtensionManager manager;

    registerBuiltInExtensions(manager);

    return manager;
}

void ExtensionManager::registerBuiltIn(ExtensionDescriptor descriptor)
{
    if (findExtension(descriptor.id) != nullptr)
    {
        SCOPE_LOG_ERROR("extension", "Duplicate extension registration: " + descriptor.id);

        return;
    }

    RegisteredExtension extension;
    extension.descriptor = std::move(descriptor);
    extension.enabled = extension.descriptor.enabledByDefault;

    m_extensions.push_back(std::move(extension));

    SCOPE_LOG_DEBUG("extension", "Registered extension: " + m_extensions.back().descriptor.id);
}

void ExtensionManager::applyConfiguration(const runtime::Configuration& configuration)
{
    for (RegisteredExtension& extension : m_extensions)
    {
        const std::string key = configurationKeyForExtension(extension.descriptor.id);

        if (!configuration.has(key))
        {
            continue;
        }

        const auto valueResult = configuration.get(key);

        if (!valueResult)
        {
            SCOPE_LOG_ERROR("extension",
                            "Invalid configuration for extension " + extension.descriptor.id + ": " +
                                valueResult.error().message());

            continue;
        }

        extension.enabled = parseEnabledValue(*valueResult);

        SCOPE_LOG_DEBUG("extension",
                        "Extension " + extension.descriptor.id +
                            (extension.enabled ? " enabled" : " disabled") + " via configuration");
    }
}

void ExtensionManager::initializeEnabled()
{
    for (RegisteredExtension& extension : m_extensions)
    {
        if (!extension.enabled || extension.initialized)
        {
            continue;
        }

        if (extension.descriptor.initialize == nullptr)
        {
            extension.initialized = true;

            continue;
        }

        const auto initResult = extension.descriptor.initialize();

        extension.initialized = true;

        if (!initResult)
        {
            extension.initializationFailed = true;

            SCOPE_LOG_ERROR("extension",
                            "Extension initialization failed for " + extension.descriptor.id + ": " +
                                initResult.error().message());
        }
    }
}

std::vector<ExtensionInfo> ExtensionManager::listExtensions() const
{
    std::vector<ExtensionInfo> extensions;
    extensions.reserve(m_extensions.size());

    for (const RegisteredExtension& extension : m_extensions)
    {
        extensions.push_back(toInfo(extension));
    }

    return extensions;
}

foundation::Result<ExtensionInfo> ExtensionManager::describe(const std::string& id) const
{
    const RegisteredExtension* extension = findExtension(id);

    if (extension == nullptr)
    {
        return foundation::Result<ExtensionInfo>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Extension not found."));
    }

    return foundation::Result<ExtensionInfo>(toInfo(*extension));
}

const ExtensionManager::RegisteredExtension* ExtensionManager::findExtension(const std::string& id) const
{
    for (const RegisteredExtension& extension : m_extensions)
    {
        if (extension.descriptor.id == id)
        {
            return &extension;
        }
    }

    return nullptr;
}

ExtensionInfo ExtensionManager::toInfo(const RegisteredExtension& extension) const noexcept
{
    ExtensionInfo info;
    info.id = extension.descriptor.id;
    info.version = extension.descriptor.version;
    info.description = extension.descriptor.description;
    info.enabled = extension.enabled;

    if (!extension.enabled)
    {
        info.status = ExtensionStatus::Disabled;
    }
    else if (extension.initializationFailed)
    {
        info.status = ExtensionStatus::InitializationFailed;
    }
    else
    {
        info.status = ExtensionStatus::Ready;
    }

    return info;
}

} // namespace scope::extension
