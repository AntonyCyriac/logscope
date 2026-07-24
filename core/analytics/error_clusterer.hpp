/**
 * @file error_clusterer.hpp
 * @brief Groups similar error messages by normalized signature.
 */

#pragma once

#include "analytics_config.hpp"
#include "analysis_model.hpp"
#include "error_cluster.hpp"

namespace scope::analytics
{

/**
 * @brief Clusters error lines using normalized message signatures.
 */
class ErrorClusterer
{
  public:
    [[nodiscard]] ClusterResult analyze(const analysis::AnalysisModel& model,
                                        const AnalyticsConfig& config) const;
};

} // namespace scope::analytics
