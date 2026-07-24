/**
 * @file error_cluster.hpp
 * @brief Error clustering result types.
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace scope::analytics
{

/**
 * @brief A group of similar error messages.
 */
struct ErrorCluster
{
    std::string signature;
    std::string sampleMessage;
    std::uint64_t count{0U};
    std::vector<std::uint64_t> lineNumbers;
};

/**
 * @brief Clustering output.
 */
class ClusterResult
{
  public:
    [[nodiscard]] const std::vector<ErrorCluster>& clusters() const noexcept;

    void setClusters(std::vector<ErrorCluster> clusters);

  private:
    std::vector<ErrorCluster> m_clusters;
};

} // namespace scope::analytics
