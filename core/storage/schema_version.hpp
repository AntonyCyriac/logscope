/**
 * @file schema_version.hpp
 * @brief Persistent index schema version constants.
 */

#pragma once

#include "foundation/error.hpp"

namespace scope::storage
{

constexpr int kIndexSchemaVersionV1 = 1;
constexpr int kIndexSchemaVersionCurrent = 2;
constexpr int kIndexSchemaVersionMaxSupported = 2;

/**
 * @brief Returns true when an open failure indicates a full rebuild from source is required.
 */
[[nodiscard]] bool requiresSchemaRebuild(const foundation::Error& error) noexcept;

} // namespace scope::storage
