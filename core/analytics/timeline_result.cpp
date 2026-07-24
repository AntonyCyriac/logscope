/**
 * @file timeline_result.cpp
 * @brief TimelineResult implementation.
 */

#include "timeline_result.hpp"

namespace scope::analytics
{

std::int64_t TimelineResult::bucketSeconds() const noexcept
{
    return m_bucketSeconds;
}

void TimelineResult::setBucketSeconds(const std::int64_t seconds) noexcept
{
    m_bucketSeconds = seconds;
}

const std::vector<TimelineBucket>& TimelineResult::buckets() const noexcept
{
    return m_buckets;
}

void TimelineResult::setBuckets(std::vector<TimelineBucket> buckets)
{
    m_buckets = std::move(buckets);
}

bool TimelineResult::hasData() const noexcept
{
    return !m_buckets.empty();
}

} // namespace scope::analytics
