/**
 * @file source_open_options.hpp
 * @brief Options for opening log sources (M14 tail support).
 */

#pragma once

#include "discovery_census.hpp"

namespace scope::source
{

/**
 * @brief Options when opening a log source.
 */
struct OpenOptions
{
    /// When true, file sources poll for appended data after EOF (live tail).
    bool follow = false;

    /// Discovery traversal options for directory and bundle sources.
    DiscoveryOptions discovery{};
};

} // namespace scope::source
