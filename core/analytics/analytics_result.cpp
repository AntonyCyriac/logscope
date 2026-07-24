/**
 * @file analytics_result.cpp
 * @brief AnalyticsResult implementation.
 */

#include "analytics_result.hpp"

namespace scope::analytics
{

const FrequencyResult& AnalyticsResult::frequency() const noexcept
{
    return m_frequency;
}

const ClusterResult& AnalyticsResult::clusters() const noexcept
{
    return m_clusters;
}

const TimelineResult& AnalyticsResult::timeline() const noexcept
{
    return m_timeline;
}

const TrendResult& AnalyticsResult::trends() const noexcept
{
    return m_trends;
}

const CorrelationResult& AnalyticsResult::correlations() const noexcept
{
    return m_correlations;
}

std::uint64_t AnalyticsResult::indexedLineCount() const noexcept
{
    return m_indexedLineCount;
}

std::uint64_t AnalyticsResult::truncatedLineCount() const noexcept
{
    return m_truncatedLineCount;
}

void AnalyticsResult::setFrequency(FrequencyResult frequency)
{
    m_frequency = std::move(frequency);
}

void AnalyticsResult::setClusters(ClusterResult clusters)
{
    m_clusters = std::move(clusters);
}

void AnalyticsResult::setTimeline(TimelineResult timeline)
{
    m_timeline = std::move(timeline);
}

void AnalyticsResult::setTrends(TrendResult trends)
{
    m_trends = std::move(trends);
}

void AnalyticsResult::setCorrelations(CorrelationResult correlations)
{
    m_correlations = std::move(correlations);
}

void AnalyticsResult::setIndexedLineCount(const std::uint64_t count) noexcept
{
    m_indexedLineCount = count;
}

void AnalyticsResult::setTruncatedLineCount(const std::uint64_t count) noexcept
{
    m_truncatedLineCount = count;
}

} // namespace scope::analytics
