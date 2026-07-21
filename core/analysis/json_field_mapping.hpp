/**
 * @file json_field_mapping.hpp
 * @brief Configurable JSON Lines field name mapping.
 */

#pragma once

#include <string>

namespace scope::analysis
{

/**
 * @brief Optional overrides for JSON Lines field extraction.
 */
struct JsonFieldMapping
{
    std::string timestampField;
    std::string levelField;

    [[nodiscard]] bool hasTimestampOverride() const noexcept;

    [[nodiscard]] bool hasLevelOverride() const noexcept;
};

} // namespace scope::analysis
