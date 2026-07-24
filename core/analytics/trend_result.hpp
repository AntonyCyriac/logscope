/**
 * @file trend_result.hpp
 * @brief Trend and spike detection output.
 */

#pragma once

#include <cstdint>
#include <string>

namespace scope::analytics
{

/**
 * @brief Summary of error-rate trends across a timeline.
 */
class TrendResult
{
  public:
    [[nodiscard]] double firstHalfErrorRate() const noexcept;

    [[nodiscard]] double secondHalfErrorRate() const noexcept;

    [[nodiscard]] double rateChangePercent() const noexcept;

    [[nodiscard]] bool hasSpike() const noexcept;

    [[nodiscard]] const std::string& verdict() const noexcept;

    void setFirstHalfErrorRate(double rate) noexcept;

    void setSecondHalfErrorRate(double rate) noexcept;

    void setRateChangePercent(double percent) noexcept;

    void setHasSpike(bool spike) noexcept;

    void setVerdict(std::string verdict);

  private:
    double m_firstHalfErrorRate{0.0};
    double m_secondHalfErrorRate{0.0};
    double m_rateChangePercent{0.0};
    bool m_hasSpike{false};
    std::string m_verdict{"Insufficient timeline data"};
};

} // namespace scope::analytics
