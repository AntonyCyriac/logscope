/**
 * @file time_range_filter.cpp
 * @brief TimeRangeFilter implementation.
 */

#include "time_range_filter.hpp"

namespace scope::investigation
{

TimeRangeFilter TimeRangeFilter::any() noexcept
{
    return TimeRangeFilter{};
}

TimeRangeFilter TimeRangeFilter::withEarliest(const foundation::Timestamp earliest) const noexcept
{
    TimeRangeFilter copy = *this;
    copy.m_earliest = earliest;

    return copy;
}

TimeRangeFilter TimeRangeFilter::withLatest(const foundation::Timestamp latest) const noexcept
{
    TimeRangeFilter copy = *this;
    copy.m_latest = latest;

    return copy;
}

bool TimeRangeFilter::isActive() const noexcept
{
    return m_earliest.has_value() || m_latest.has_value();
}

bool TimeRangeFilter::matches(const analysis::IndexedLine& line) const noexcept
{
    if (!isActive())
    {
        return true;
    }

    if (!line.timestamp.has_value())
    {
        return false;
    }

    if (m_earliest.has_value() && line.timestamp->unixNanoseconds() < m_earliest->unixNanoseconds())
    {
        return false;
    }

    if (m_latest.has_value() && line.timestamp->unixNanoseconds() > m_latest->unixNanoseconds())
    {
        return false;
    }

    return true;
}

const std::optional<foundation::Timestamp>& TimeRangeFilter::earliest() const noexcept
{
    return m_earliest;
}

const std::optional<foundation::Timestamp>& TimeRangeFilter::latest() const noexcept
{
    return m_latest;
}

} // namespace scope::investigation
