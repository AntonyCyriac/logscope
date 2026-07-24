/**
 * @file frequency_result.cpp
 * @brief FrequencyResult implementation.
 */

#include "frequency_result.hpp"

namespace scope::analytics
{

const analysis::LogLevelCounts& FrequencyResult::levelCounts() const noexcept
{
    return m_levelCounts;
}

void FrequencyResult::setLevelCounts(const analysis::LogLevelCounts counts) noexcept
{
    m_levelCounts = counts;
}

const std::vector<FrequencyEntry>& FrequencyResult::topMessages() const noexcept
{
    return m_topMessages;
}

void FrequencyResult::setTopMessages(std::vector<FrequencyEntry> entries)
{
    m_topMessages = std::move(entries);
}

const std::vector<FrequencyEntry>& FrequencyResult::topCorrelationIds() const noexcept
{
    return m_topCorrelationIds;
}

void FrequencyResult::setTopCorrelationIds(std::vector<FrequencyEntry> entries)
{
    m_topCorrelationIds = std::move(entries);
}

} // namespace scope::analytics
