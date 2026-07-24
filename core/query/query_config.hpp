/**
 * @file query_config.hpp
 * @brief Query-related configuration validation.
 */

#pragma once

#include "foundation/result.hpp"
#include "runtime/configuration.hpp"

namespace scope::query
{

/// Prefix for named saved filter configuration keys.
constexpr const char* savedFilterKeyPrefix = "query.saved.";

/**
 * @brief Validates query-related configuration keys and values.
 */
[[nodiscard]] foundation::Result<bool> validateQueryConfiguration(
    const runtime::Configuration& configuration) noexcept;

} // namespace scope::query
