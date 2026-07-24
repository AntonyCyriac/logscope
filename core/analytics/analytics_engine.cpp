/**
 * @file analytics_engine.cpp
 * @brief AnalyticsEngine implementation.
 */

#include "analytics_engine.hpp"

#include "correlation_analyzer.hpp"
#include "error_clusterer.hpp"
#include "frequency_analyzer.hpp"
#include "timeline_analyzer.hpp"
#include "trend_analyzer.hpp"
#include "indexed_line_access.hpp"

namespace scope::analytics
{

AnalyticsResult AnalyticsEngine::analyze(const analysis::AnalysisModel& model,
                                         const AnalyticsConfig& config) const
{
    AnalyticsResult result;

    if (analysis::hasQueryableIndex(model))
    {
        result.setIndexedLineCount(analysis::indexedLineCountForModel(model));
        result.setTruncatedLineCount(analysis::truncatedLineCountForModel(model));
    }

    const FrequencyAnalyzer frequencyAnalyzer;
    result.setFrequency(frequencyAnalyzer.analyze(model, config));

    const ErrorClusterer errorClusterer;
    result.setClusters(errorClusterer.analyze(model, config));

    TimelineResult timeline;

    if (config.includeTimeline)
    {
        const TimelineAnalyzer timelineAnalyzer;
        timeline = timelineAnalyzer.analyze(model, config);
    }

    result.setTimeline(timeline);

    const TrendAnalyzer trendAnalyzer;
    result.setTrends(trendAnalyzer.analyze(timeline));

    const CorrelationAnalyzer correlationAnalyzer;
    result.setCorrelations(correlationAnalyzer.analyze(model));

    return result;
}

} // namespace scope::analytics
