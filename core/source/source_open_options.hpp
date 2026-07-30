/**
 * @file source_open_options.hpp
 * @brief Options for opening log sources (M14 tail support).
 */

#pragma once

namespace scope::source
{

/**
 * @brief Options when opening a log source.
 */
struct OpenOptions
{
    /// When true, file sources poll for appended data after EOF (live tail).
    bool follow = false;
};

} // namespace scope::source
