/**
 * @file trend_result.cpp
 * @brief TrendResult implementation.
 */

#include "trend_result.hpp"

namespace scope::analytics
{

double TrendResult::firstHalfErrorRate() const noexcept
{
    return m_firstHalfErrorRate;
}

double TrendResult::secondHalfErrorRate() const noexcept
{
    return m_secondHalfErrorRate;
}

double TrendResult::rateChangePercent() const noexcept
{
    return m_rateChangePercent;
}

bool TrendResult::hasSpike() const noexcept
{
    return m_hasSpike;
}

const std::string& TrendResult::verdict() const noexcept
{
    return m_verdict;
}

void TrendResult::setFirstHalfErrorRate(const double rate) noexcept
{
    m_firstHalfErrorRate = rate;
}

void TrendResult::setSecondHalfErrorRate(const double rate) noexcept
{
    m_secondHalfErrorRate = rate;
}

void TrendResult::setRateChangePercent(const double percent) noexcept
{
    m_rateChangePercent = percent;
}

void TrendResult::setHasSpike(const bool spike) noexcept
{
    m_hasSpike = spike;
}

void TrendResult::setVerdict(std::string verdict)
{
    m_verdict = std::move(verdict);
}

} // namespace scope::analytics
