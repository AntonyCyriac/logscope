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

namespace scope::analytics
{

AnalyticsResult AnalyticsEngine::analyze(const analysis::AnalysisModel& model,
                                         const AnalyticsConfig& config) const
{
    AnalyticsResult result;

    if (model.lineIndex().has_value())
    {
        result.setIndexedLineCount(model.lineIndex()->indexedLineCount());
        result.setTruncatedLineCount(model.lineIndex()->truncatedLineCount());
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
