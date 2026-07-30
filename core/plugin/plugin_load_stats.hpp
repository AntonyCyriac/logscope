/**
 * @file plugin_load_stats.hpp
 * @brief Plugin discovery and load metrics (Phase 1 observability).
 */

#pragma once

#include <cstddef>

#include "foundation/duration.hpp"

namespace scope::plugin
{

/**
 * @brief Counts and timing for dynamic plugin loading.
 */
struct PluginLoadStats
{
    bool attempted{false};
    std::size_t loaded{0U};
    std::size_t skippedPaths{0U};
    std::size_t failed{0U};
    foundation::Duration loadDuration{};
};

} // namespace scope::plugin
