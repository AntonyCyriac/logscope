/**
 * @file error_clusterer.cpp
 * @brief ErrorClusterer implementation.
 */

#include "error_clusterer.hpp"

#include <algorithm>
#include <unordered_map>

#include "log_line_classifier.hpp"
#include "message_signature.hpp"

namespace scope::analytics
{

ClusterResult ErrorClusterer::analyze(const analysis::AnalysisModel& model,
                                      const AnalyticsConfig& config) const
{
    ClusterResult result;

    if (!model.lineIndex().has_value())
    {
        return result;
    }

    std::unordered_map<std::string, ErrorCluster> clusters;

    for (const analysis::IndexedLine& line : model.lineIndex()->lines())
    {
        if (line.level != analysis::DetectedLogLevel::Error || line.messageExcerpt.empty())
        {
            continue;
        }

        const std::string signature = normalizeClusterSignature(line.messageExcerpt);

        if (signature.empty())
        {
            continue;
        }

        ErrorCluster& cluster = clusters[signature];

        if (cluster.signature.empty())
        {
            cluster.signature = signature;
            cluster.sampleMessage = line.messageExcerpt;
        }

        ++cluster.count;
        cluster.lineNumbers.push_back(line.lineNumber);
    }

    std::vector<ErrorCluster> ranked;
    ranked.reserve(clusters.size());

    for (auto& entry : clusters)
    {
        if (entry.second.count >= config.minClusterCount)
        {
            ranked.push_back(std::move(entry.second));
        }
    }

    std::sort(ranked.begin(),
              ranked.end(),
              [](const ErrorCluster& left, const ErrorCluster& right) {
                  if (left.count != right.count)
                  {
                      return left.count > right.count;
                  }

                  return left.signature < right.signature;
              });

    if (ranked.size() > config.topN)
    {
        ranked.resize(config.topN);
    }

    result.setClusters(std::move(ranked));

    return result;
}

} // namespace scope::analytics
