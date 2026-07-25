/**
 * @file extension_descriptor.hpp
 * @brief Registration descriptor for built-in extensions.
 */

#pragma once

#include <cstdint>
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

    bool dynamic = false;

    std::uint32_t apiVersion = 0U;

    std::string libraryPath;
};

} // namespace scope::extension
