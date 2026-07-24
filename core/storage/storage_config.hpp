/**
 * @file storage_config.hpp
 * @brief Storage behaviour resolved from configuration.
 */

#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

#include "foundation/path.hpp"
#include "foundation/result.hpp"
#include "runtime/configuration.hpp"

namespace scope::storage
{

enum class StorageMode
{
    Memory,
    Hybrid,
    Persistent
};

/**
 * @brief Resolved storage settings for a single run.
 */
struct StorageConfig
{
    StorageMode mode{StorageMode::Memory};
    bool persistIndex{false};
    bool reuseIndex{false};
    std::optional<foundation::Path> indexPath;
    foundation::Path indexDirectory;
    std::size_t spillThreshold{0U};

    [[nodiscard]] bool usesPersistentStore() const noexcept;

    [[nodiscard]] static StorageConfig defaults() noexcept;
};

[[nodiscard]] StorageConfig resolveStorageConfig(const runtime::Configuration& configuration,
                                                 const StorageConfig& cliOverrides) noexcept;

[[nodiscard]] foundation::Result<bool> validateStorageConfiguration(
    const runtime::Configuration& configuration) noexcept;

[[nodiscard]] std::string_view storageModeName(StorageMode mode) noexcept;

[[nodiscard]] std::optional<StorageMode> parseStorageMode(std::string_view value) noexcept;

} // namespace scope::storage
