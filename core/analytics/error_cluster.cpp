/**
 * @file error_cluster.cpp
 * @brief ClusterResult implementation.
 */

#include "error_cluster.hpp"

namespace scope::analytics
{

const std::vector<ErrorCluster>& ClusterResult::clusters() const noexcept
{
    return m_clusters;
}

void ClusterResult::setClusters(std::vector<ErrorCluster> clusters)
{
    m_clusters = std::move(clusters);
}

} // namespace scope::analytics
