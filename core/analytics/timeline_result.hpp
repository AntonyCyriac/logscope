/**
 * @file timeline_result.hpp
 * @brief Timeline bucket analysis output.
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "foundation/timestamp.hpp"
#include "log_level_counts.hpp"

namespace scope::analytics
{

/**
 * @brief Per-level counts within a time bucket.
 */
struct TimelineBucket
{
    std::string label;
    foundation::Timestamp start;
    foundation::Timestamp end;
    analysis::LogLevelCounts levelCounts;
    std::uint64_t totalLines{0U};
};

/**
 * @brief Timeline analysis output.
 */
class TimelineResult
{
  public:
    [[nodiscard]] std::int64_t bucketSeconds() const noexcept;

    void setBucketSeconds(const std::int64_t seconds) noexcept;

    [[nodiscard]] const std::vector<TimelineBucket>& buckets() const noexcept;

    void setBuckets(std::vector<TimelineBucket> buckets);

    [[nodiscard]] bool hasData() const noexcept;

  private:
    std::int64_t m_bucketSeconds{0};
    std::vector<TimelineBucket> m_buckets;
};

} // namespace scope::analytics
