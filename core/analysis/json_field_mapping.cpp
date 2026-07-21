/**
 * @file json_field_mapping.cpp
 * @brief JsonFieldMapping implementation.
 */

#include "json_field_mapping.hpp"

namespace scope::analysis
{

bool JsonFieldMapping::hasTimestampOverride() const noexcept
{
    return !timestampField.empty();
}

bool JsonFieldMapping::hasLevelOverride() const noexcept
{
    return !levelField.empty();
}

} // namespace scope::analysis
