/**
 * @file correlation_result.cpp
 * @brief CorrelationResult implementation.
 */

#include "correlation_result.hpp"

namespace scope::analytics
{

const std::vector<CorrelationEntry>& CorrelationResult::repeatedErrors() const noexcept
{
    return m_repeatedErrors;
}

const std::vector<CorrelationEntry>& CorrelationResult::sharedCorrelationIds() const noexcept
{
    return m_sharedCorrelationIds;
}

void CorrelationResult::setRepeatedErrors(std::vector<CorrelationEntry> entries)
{
    m_repeatedErrors = std::move(entries);
}

void CorrelationResult::setSharedCorrelationIds(std::vector<CorrelationEntry> entries)
{
    m_sharedCorrelationIds = std::move(entries);
}

} // namespace scope::analytics
