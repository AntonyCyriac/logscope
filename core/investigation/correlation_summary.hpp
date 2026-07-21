/**
 * @file correlation_summary.hpp
 * @brief Repeated errors and shared correlation identifiers.
 */

#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace scope::investigation
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
class CorrelationSummary
{
  public:
    [[nodiscard]] const std::vector<CorrelationEntry>& repeatedErrors() const noexcept;

    [[nodiscard]] const std::vector<CorrelationEntry>& sharedCorrelationIds() const noexcept;

    void addRepeatedError(CorrelationEntry entry);

    void addSharedCorrelationId(CorrelationEntry entry);

  private:
    std::vector<CorrelationEntry> m_repeatedErrors;
    std::vector<CorrelationEntry> m_sharedCorrelationIds;
};

} // namespace scope::investigation
