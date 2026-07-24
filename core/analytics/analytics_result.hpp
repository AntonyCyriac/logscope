/**
 * @file analytics_result.hpp
 * @brief Combined analytics output.
 */

#pragma once

#include <cstdint>

#include "error_cluster.hpp"
#include "correlation_result.hpp"
#include "frequency_result.hpp"
#include "timeline_result.hpp"
#include "trend_result.hpp"

namespace scope::analytics
{

/**
 * @brief Bundled analytics findings for an analysis model.
 */
class AnalyticsResult
{
  public:
    [[nodiscard]] const FrequencyResult& frequency() const noexcept;

    [[nodiscard]] const ClusterResult& clusters() const noexcept;

    [[nodiscard]] const TimelineResult& timeline() const noexcept;

    [[nodiscard]] const TrendResult& trends() const noexcept;

    [[nodiscard]] const CorrelationResult& correlations() const noexcept;

    [[nodiscard]] std::uint64_t indexedLineCount() const noexcept;

    [[nodiscard]] std::uint64_t truncatedLineCount() const noexcept;

    void setFrequency(FrequencyResult frequency);

    void setClusters(ClusterResult clusters);

    void setTimeline(TimelineResult timeline);

    void setTrends(TrendResult trends);

    void setCorrelations(CorrelationResult correlations);

    void setIndexedLineCount(std::uint64_t count) noexcept;

    void setTruncatedLineCount(std::uint64_t count) noexcept;

  private:
    FrequencyResult m_frequency;
    ClusterResult m_clusters;
    TimelineResult m_timeline;
    TrendResult m_trends;
    CorrelationResult m_correlations;
    std::uint64_t m_indexedLineCount{0U};
    std::uint64_t m_truncatedLineCount{0U};
};

} // namespace scope::analytics
