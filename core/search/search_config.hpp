/**
 * @file search_config.hpp
 * @brief Search-related configuration validation.
 */

#pragma once

#include "foundation/result.hpp"
#include "runtime/configuration.hpp"

namespace scope::search
{

/// Prefix for named saved search configuration keys.
constexpr const char* savedSearchKeyPrefix = "search.saved.";

/**
 * @brief Validates search-related configuration keys and values.
 */
[[nodiscard]] foundation::Result<bool> validateSearchConfiguration(
    const runtime::Configuration& configuration) noexcept;

} // namespace scope::search
