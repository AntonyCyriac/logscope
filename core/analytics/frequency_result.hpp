/**
 * @file frequency_result.hpp
 * @brief Frequency analysis output types.
 */

#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "log_level_counts.hpp"

namespace scope::analytics
{

/**
 * @brief A counted frequency entry.
 */
struct FrequencyEntry
{
    std::string key;
    std::uint64_t count{0U};
};

/**
 * @brief Frequency tables derived from indexed lines.
 */
class FrequencyResult
{
  public:
    [[nodiscard]] const analysis::LogLevelCounts& levelCounts() const noexcept;

    void setLevelCounts(analysis::LogLevelCounts counts) noexcept;

    [[nodiscard]] const std::vector<FrequencyEntry>& topMessages() const noexcept;

    void setTopMessages(std::vector<FrequencyEntry> entries);

    [[nodiscard]] const std::vector<FrequencyEntry>& topCorrelationIds() const noexcept;

    void setTopCorrelationIds(std::vector<FrequencyEntry> entries);

  private:
    analysis::LogLevelCounts m_levelCounts;
    std::vector<FrequencyEntry> m_topMessages;
    std::vector<FrequencyEntry> m_topCorrelationIds;
};

} // namespace scope::analytics
