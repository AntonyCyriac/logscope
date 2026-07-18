/**
 * @file log_level_counts.cpp
 * @brief LogLevelCounts implementation.
 */

#include "log_level_counts.hpp"

namespace scope::analysis
{

std::uint64_t LogLevelCounts::infoLines() const noexcept
{
    return m_infoLines;
}

std::uint64_t LogLevelCounts::warnLines() const noexcept
{
    return m_warnLines;
}

std::uint64_t LogLevelCounts::errorLines() const noexcept
{
    return m_errorLines;
}

std::uint64_t LogLevelCounts::otherLines() const noexcept
{
    return m_otherLines;
}

std::uint64_t LogLevelCounts::blankLines() const noexcept
{
    return m_blankLines;
}

void LogLevelCounts::recordInfo() noexcept
{
    ++m_infoLines;
}

void LogLevelCounts::recordWarn() noexcept
{
    ++m_warnLines;
}

void LogLevelCounts::recordError() noexcept
{
    ++m_errorLines;
}

void LogLevelCounts::recordOther() noexcept
{
    ++m_otherLines;
}

void LogLevelCounts::recordBlank() noexcept
{
    ++m_blankLines;
}

LogLevelCounts LogLevelCounts::fromCounts(const std::uint64_t infoLines,
                                          const std::uint64_t warnLines,
                                          const std::uint64_t errorLines,
                                          const std::uint64_t otherLines,
                                          const std::uint64_t blankLines) noexcept
{
    LogLevelCounts counts;
    counts.m_infoLines = infoLines;
    counts.m_warnLines = warnLines;
    counts.m_errorLines = errorLines;
    counts.m_otherLines = otherLines;
    counts.m_blankLines = blankLines;

    return counts;
}

} // namespace scope::analysis
