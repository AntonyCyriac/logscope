/**
 * @file ai_analytics_context_builder.cpp
 */

#include "ai_analytics_context_builder.hpp"

namespace scope::ai
{

namespace
{

std::string clusterMessage(const analytics::ErrorCluster& cluster)
{
    if (!cluster.sampleMessage.empty())
    {
        return cluster.sampleMessage;
    }

    return cluster.signature;
}

} // namespace

AiAnalyticsContext buildAnalyticsContext(const analytics::AnalyticsResult& analytics)
{
    AiAnalyticsContext context;

    const analytics::TrendResult& trends = analytics.trends();
    context.hasSpike = trends.hasSpike();
    context.spikeVerdict = trends.verdict();

    const auto& clusters = analytics.clusters().clusters();
    context.clusterCount = clusters.size();

    if (!clusters.empty())
    {
        context.topClusterMessage = clusterMessage(clusters[0]);
        context.topClusterCount = clusters[0].count;
    }

    const auto& repeatedErrors = analytics.correlations().repeatedErrors();
    context.repeatedErrorPatternCount = repeatedErrors.size();

    if (!repeatedErrors.empty())
    {
        context.topRepeatedErrorKey = repeatedErrors[0].key;
        context.topRepeatedErrorCount = repeatedErrors[0].count;
    }

    return context;
}

} // namespace scope::ai
