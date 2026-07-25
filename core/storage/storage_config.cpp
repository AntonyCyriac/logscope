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
constexpr std::string_view storageCompressContentKey = "storage.compress_content";
constexpr std::string_view storageCompressThresholdKey = "storage.compress_threshold_bytes";
constexpr std::string_view storageQueryCacheEnabledKey = "storage.query_cache.enabled";
constexpr std::string_view storageQueryCacheMaxEntriesKey = "storage.query_cache.max_entries";
constexpr std::string_view storageIncrementalAppendKey = "storage.incremental_append";
constexpr std::string_view storageBackendKey = "storage.backend";

[[nodiscard]] std::optional<bool> parseBooleanConfig(std::string_view value) noexcept
{
    const std::string lowered = foundation::toLower(value);

    if (lowered == "true" || lowered == "1" || lowered == "on" || lowered == "yes")
    {
        return true;
    }

    if (lowered == "false" || lowered == "0" || lowered == "off" || lowered == "no")
    {
        return false;
    }

    return std::nullopt;
}

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

    if (configuration.has(std::string(storageCompressContentKey)))
    {
        const auto compressContent = configuration.get(std::string(storageCompressContentKey));

        if (compressContent && !foundation::isBlank(*compressContent))
        {
            const auto parsed = parseBooleanConfig(*compressContent);

            if (parsed.has_value())
            {
                config.compressContent = *parsed;
            }
        }
    }

    if (configuration.has(std::string(storageCompressThresholdKey)))
    {
        const auto compressThreshold = configuration.get(std::string(storageCompressThresholdKey));

        if (compressThreshold && !foundation::isBlank(*compressThreshold))
        {
            config.compressThresholdBytes = static_cast<std::size_t>(std::stoull(*compressThreshold));
        }
    }

    if (configuration.has(std::string(storageQueryCacheEnabledKey)))
    {
        const auto queryCacheEnabled = configuration.get(std::string(storageQueryCacheEnabledKey));

        if (queryCacheEnabled && !foundation::isBlank(*queryCacheEnabled))
        {
            const auto parsed = parseBooleanConfig(*queryCacheEnabled);

            if (parsed.has_value())
            {
                config.queryCacheEnabled = *parsed;
            }
        }
    }

    if (configuration.has(std::string(storageQueryCacheMaxEntriesKey)))
    {
        const auto queryCacheMaxEntries = configuration.get(std::string(storageQueryCacheMaxEntriesKey));

        if (queryCacheMaxEntries && !foundation::isBlank(*queryCacheMaxEntries))
        {
            config.queryCacheMaxEntries = static_cast<std::size_t>(std::stoull(*queryCacheMaxEntries));
        }
    }

    if (configuration.has(std::string(storageIncrementalAppendKey)))
    {
        const auto incrementalAppend = configuration.get(std::string(storageIncrementalAppendKey));

        if (incrementalAppend && !foundation::isBlank(*incrementalAppend))
        {
            const auto parsed = parseBooleanConfig(*incrementalAppend);

            if (parsed.has_value())
            {
                config.incrementalAppend = *parsed;
            }
        }
    }

    if (configuration.has(std::string(storageBackendKey)))
    {
        const auto backend = configuration.get(std::string(storageBackendKey));

        if (backend && !foundation::isBlank(*backend))
        {
            config.backend = *backend;
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

    if (cliOverrides.compressContent)
    {
        config.compressContent = true;
    }

    if (cliOverrides.compressThresholdBytes != StorageConfig::defaults().compressThresholdBytes)
    {
        config.compressThresholdBytes = cliOverrides.compressThresholdBytes;
    }

    if (!cliOverrides.queryCacheEnabled)
    {
        config.queryCacheEnabled = false;
    }

    if (cliOverrides.queryCacheMaxEntries != StorageConfig::defaults().queryCacheMaxEntries)
    {
        config.queryCacheMaxEntries = cliOverrides.queryCacheMaxEntries;
    }

    if (!cliOverrides.incrementalAppend)
    {
        config.incrementalAppend = false;
    }

    if (!cliOverrides.backend.empty() && cliOverrides.backend != "sqlite")
    {
        config.backend = cliOverrides.backend;
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

    if (const auto compressContent = configuration.get(std::string(storageCompressContentKey));
        compressContent && !foundation::isBlank(*compressContent))
    {
        if (!parseBooleanConfig(*compressContent).has_value())
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::InvalidArgument, "Invalid storage.compress_content value."));
        }
    }

    if (const auto compressThreshold = configuration.get(std::string(storageCompressThresholdKey));
        compressThreshold && !foundation::isBlank(*compressThreshold))
    {
        try
        {
            (void)std::stoull(*compressThreshold);
        }
        catch (...)
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::InvalidArgument, "Invalid storage.compress_threshold_bytes value."));
        }
    }

    if (const auto queryCacheEnabled = configuration.get(std::string(storageQueryCacheEnabledKey));
        queryCacheEnabled && !foundation::isBlank(*queryCacheEnabled))
    {
        if (!parseBooleanConfig(*queryCacheEnabled).has_value())
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::InvalidArgument, "Invalid storage.query_cache.enabled value."));
        }
    }

    if (const auto queryCacheMaxEntries = configuration.get(std::string(storageQueryCacheMaxEntriesKey));
        queryCacheMaxEntries && !foundation::isBlank(*queryCacheMaxEntries))
    {
        try
        {
            (void)std::stoull(*queryCacheMaxEntries);
        }
        catch (...)
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::InvalidArgument, "Invalid storage.query_cache.max_entries value."));
        }
    }

    if (const auto incrementalAppend = configuration.get(std::string(storageIncrementalAppendKey));
        incrementalAppend && !foundation::isBlank(*incrementalAppend))
    {
        if (!parseBooleanConfig(*incrementalAppend).has_value())
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::InvalidArgument, "Invalid storage.incremental_append value."));
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
