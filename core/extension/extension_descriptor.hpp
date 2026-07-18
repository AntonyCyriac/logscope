/**
 * @file extension_descriptor.hpp
 * @brief Registration descriptor for built-in extensions.
 */

#pragma once

#include <string>

#include "foundation/result.hpp"

namespace scope::extension
{

/**
 * @brief Optional initialization hook for an extension.
 */
using ExtensionInitializeFn = foundation::Result<bool> (*)();

/**
 * @brief Describes a built-in extension at registration time.
 */
struct ExtensionDescriptor
{
    std::string id;

    std::string version;

    std::string description;

    ExtensionInitializeFn initialize = nullptr;

    bool enabledByDefault = true;
};

} // namespace scope::extension
