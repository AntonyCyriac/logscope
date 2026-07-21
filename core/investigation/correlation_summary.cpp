/**
 * @file correlation_summary.cpp
 * @brief CorrelationSummary implementation.
 */

#include "correlation_summary.hpp"

namespace scope::investigation
{

const std::vector<CorrelationEntry>& CorrelationSummary::repeatedErrors() const noexcept
{
    return m_repeatedErrors;
}

const std::vector<CorrelationEntry>& CorrelationSummary::sharedCorrelationIds() const noexcept
{
    return m_sharedCorrelationIds;
}

void CorrelationSummary::addRepeatedError(CorrelationEntry entry)
{
    m_repeatedErrors.push_back(std::move(entry));
}

void CorrelationSummary::addSharedCorrelationId(CorrelationEntry entry)
{
    m_sharedCorrelationIds.push_back(std::move(entry));
}

} // namespace scope::investigation
