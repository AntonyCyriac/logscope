/**
 * @file correlation_result.hpp
 * @brief Correlation findings from analytics.
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace scope::analytics
{

/**
 * @brief A repeated pattern surfaced during correlation analysis.
 */
struct CorrelationEntry
{
    std::string key;
    std::uint64_t count{0U};
    std::vector<std::uint64_t> lineNumbers;
};

/**
 * @brief Correlation findings from indexed log lines.
 */
class CorrelationResult
{
  public:
    [[nodiscard]] const std::vector<CorrelationEntry>& repeatedErrors() const noexcept;

    [[nodiscard]] const std::vector<CorrelationEntry>& sharedCorrelationIds() const noexcept;

    void setRepeatedErrors(std::vector<CorrelationEntry> entries);

    void setSharedCorrelationIds(std::vector<CorrelationEntry> entries);

  private:
    std::vector<CorrelationEntry> m_repeatedErrors;
    std::vector<CorrelationEntry> m_sharedCorrelationIds;
};

} // namespace scope::analytics
