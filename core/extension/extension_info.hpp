/**
 * @file extension_info.hpp
 * @brief Describes a registered extension.
 */

#pragma once

#include <string>

namespace scope::extension
{

/**
 * @brief Lifecycle status of an extension.
 */
enum class ExtensionStatus
{
    Ready,
    Disabled,
    InitializationFailed
};

/**
 * @brief Public metadata for a registered extension.
 */
struct ExtensionInfo
{
    std::string id;

    std::string version;

    std::string description;

    bool enabled = true;

    ExtensionStatus status = ExtensionStatus::Ready;
};

} // namespace scope::extension
