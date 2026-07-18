/**
 * @file line_count_filter.cpp
 * @brief LineCountFilter implementation.
 */

#include "line_count_filter.hpp"

namespace scope::investigation
{

LineCountFilter LineCountFilter::any() noexcept
{
    return LineCountFilter{};
}

LineCountFilter LineCountFilter::nonEmpty() noexcept
{
    return LineCountFilter{}.withMinimum(1U);
}

LineCountFilter LineCountFilter::withMinimum(const std::uint64_t minLines) const noexcept
{
    LineCountFilter refined = *this;
    refined.m_minLines = minLines;

    return refined;
}

LineCountFilter LineCountFilter::withMaximum(const std::uint64_t maxLines) const noexcept
{
    LineCountFilter refined = *this;
    refined.m_maxLines = maxLines;

    return refined;
}

bool LineCountFilter::matches(const analysis::AnalysisModel& model) const noexcept
{
    const std::uint64_t totalLines = model.totalLines();

    return totalLines >= m_minLines && totalLines <= m_maxLines;
}

std::uint64_t LineCountFilter::minimumLines() const noexcept
{
    return m_minLines;
}

std::uint64_t LineCountFilter::maximumLines() const noexcept
{
    return m_maxLines;
}

bool LineCountFilter::hasMaximumLines() const noexcept
{
    return m_maxLines < std::numeric_limits<std::uint64_t>::max();
}

} // namespace scope::investigation
