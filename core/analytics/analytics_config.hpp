/**
 * @file analytics_config.hpp
 * @brief Configuration for analytics engines.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

namespace scope::analytics
{

/// Default number of top frequency/cluster results.
constexpr std::size_t defaultTopN = 10U;

/// Default minimum occurrences to surface a cluster.
constexpr std::uint64_t defaultMinClusterCount = 2U;

/// Target number of timeline buckets when auto-sizing.
constexpr std::size_t defaultTimelineBucketCount = 12U;

/**
 * @brief Controls analytics behavior.
 */
struct AnalyticsConfig
{
    std::optional<std::int64_t> bucketSeconds;
    std::size_t topN = defaultTopN;
    std::uint64_t minClusterCount = defaultMinClusterCount;
    bool includeTimeline = true;
};

} // namespace scope::analytics
