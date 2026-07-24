/**
 * @file storage_config.cpp
 */

#include "storage_config.hpp"

#include "foundation/string.hpp"
#include "workspace_paths.hpp"

namespace scope::storage
{

namespace
{

constexpr std::string_view storageModeKey = "storage.mode";
constexpr std::string_view storageIndexDirectoryKey = "storage.index.directory";
constexpr std::string_view storageIndexPathKey = "storage.index.path";
constexpr std::string_view storageSpillThresholdKey = "storage.spill_threshold";

void applyModeOverride(StorageConfig& config, const std::string& modeValue) noexcept
{
    if (foundation::isBlank(modeValue))
    {
        return;
    }

    const auto parsed = parseStorageMode(modeValue);

    if (parsed.has_value())
    {
        config.mode = *parsed;
    }
}

} // namespace

bool StorageConfig::usesPersistentStore() const noexcept
{
    return persistIndex || mode == StorageMode::Hybrid || mode == StorageMode::Persistent ||
           indexPath.has_value();
}

StorageConfig StorageConfig::defaults() noexcept
{
    StorageConfig config;
    config.indexDirectory = defaultIndexDirectory();

    return config;
}

StorageConfig resolveStorageConfig(const runtime::Configuration& configuration,
                                   const StorageConfig& cliOverrides) noexcept
{
    StorageConfig config = StorageConfig::defaults();

    if (configuration.has(std::string(storageModeKey)))
    {
        const auto modeValue = configuration.get(std::string(storageModeKey));

        if (modeValue)
        {
            applyModeOverride(config, *modeValue);
        }
    }

    if (configuration.has(std::string(storageIndexDirectoryKey)))
    {
        const auto directory = configuration.get(std::string(storageIndexDirectoryKey));

        if (directory && !foundation::isBlank(*directory))
        {
            config.indexDirectory = foundation::Path(*directory);
        }
    }

    if (configuration.has(std::string(storageIndexPathKey)))
    {
        const auto indexPath = configuration.get(std::string(storageIndexPathKey));

        if (indexPath && !foundation::isBlank(*indexPath))
        {
            config.indexPath = foundation::Path(*indexPath);
            config.mode = StorageMode::Persistent;
        }
    }

    if (configuration.has(std::string(storageSpillThresholdKey)))
    {
        const auto spillThreshold = configuration.get(std::string(storageSpillThresholdKey));

        if (spillThreshold && !foundation::isBlank(*spillThreshold))
        {
            config.spillThreshold = static_cast<std::size_t>(std::stoull(*spillThreshold));
        }
    }

    if (cliOverrides.mode != StorageMode::Memory)
    {
        config.mode = cliOverrides.mode;
    }

    if (cliOverrides.persistIndex)
    {
        config.persistIndex = true;

        if (config.mode == StorageMode::Memory)
        {
            config.mode = StorageMode::Hybrid;
        }
    }

    if (cliOverrides.reuseIndex)
    {
        config.reuseIndex = true;
    }

    if (cliOverrides.indexPath.has_value())
    {
        config.indexPath = cliOverrides.indexPath;
        config.mode = StorageMode::Persistent;
    }

    if (!cliOverrides.indexDirectory.string().empty())
    {
        config.indexDirectory = cliOverrides.indexDirectory;
    }

    if (cliOverrides.spillThreshold > 0U)
    {
        config.spillThreshold = cliOverrides.spillThreshold;
    }

    return config;
}

foundation::Result<bool> validateStorageConfiguration(const runtime::Configuration& configuration) noexcept
{
    if (const auto mode = configuration.get(std::string(storageModeKey)); mode && !foundation::isBlank(*mode))
    {
        if (!parseStorageMode(*mode).has_value())
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::InvalidArgument,
                "Invalid storage.mode value. Use memory, hybrid, or persistent."));
        }
    }

    if (const auto spillThreshold = configuration.get(std::string(storageSpillThresholdKey));
        spillThreshold && !foundation::isBlank(*spillThreshold))
    {
        try
        {
            (void)std::stoull(*spillThreshold);
        }
        catch (...)
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::InvalidArgument, "Invalid storage.spill_threshold value."));
        }
    }

    return foundation::Result<bool>(true);
}

std::string_view storageModeName(StorageMode mode) noexcept
{
    switch (mode)
    {
    case StorageMode::Memory:
        return "memory";
    case StorageMode::Hybrid:
        return "hybrid";
    case StorageMode::Persistent:
        return "persistent";
    }

    return "memory";
}

std::optional<StorageMode> parseStorageMode(std::string_view value) noexcept
{
    const std::string lowered = foundation::toLower(value);

    if (lowered == "memory")
    {
        return StorageMode::Memory;
    }

    if (lowered == "hybrid")
    {
        return StorageMode::Hybrid;
    }

    if (lowered == "persistent")
    {
        return StorageMode::Persistent;
    }

    return std::nullopt;
}

} // namespace scope::storage
