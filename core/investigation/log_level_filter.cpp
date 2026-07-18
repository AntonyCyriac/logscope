/**
 * @file log_level_filter.cpp
 * @brief LogLevelFilter implementation.
 */

#include "log_level_filter.hpp"

namespace scope::investigation
{

LogLevelFilter LogLevelFilter::any() noexcept
{
    return LogLevelFilter{};
}

LogLevelFilter LogLevelFilter::withMinimumErrors(const std::uint64_t minErrors) const noexcept
{
    LogLevelFilter refined = *this;
    refined.m_minErrors = minErrors;

    return refined;
}

LogLevelFilter LogLevelFilter::withMinimumWarnings(const std::uint64_t minWarnings) const noexcept
{
    LogLevelFilter refined = *this;
    refined.m_minWarnings = minWarnings;

    return refined;
}

bool LogLevelFilter::matches(const analysis::AnalysisModel& model) const noexcept
{
    const analysis::LogLevelCounts& counts = model.levelCounts();

    return counts.errorLines() >= m_minErrors && counts.warnLines() >= m_minWarnings;
}

std::uint64_t LogLevelFilter::minimumErrors() const noexcept
{
    return m_minErrors;
}

std::uint64_t LogLevelFilter::minimumWarnings() const noexcept
{
    return m_minWarnings;
}

} // namespace scope::investigation
