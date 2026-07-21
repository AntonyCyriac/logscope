/**
 * @file time_range_filter.hpp
 * @brief Filters indexed lines by timestamp range.
 */

#pragma once

#include <optional>

#include "foundation/timestamp.hpp"
#include "line_index.hpp"

namespace scope::investigation
{

/**
 * @brief Progressive timestamp range filter for indexed log lines.
 */
class TimeRangeFilter
{
  public:
    [[nodiscard]] static TimeRangeFilter any() noexcept;

    [[nodiscard]] TimeRangeFilter withEarliest(foundation::Timestamp earliest) const noexcept;

    [[nodiscard]] TimeRangeFilter withLatest(foundation::Timestamp latest) const noexcept;

    [[nodiscard]] bool isActive() const noexcept;

    [[nodiscard]] bool matches(const analysis::IndexedLine& line) const noexcept;

    [[nodiscard]] const std::optional<foundation::Timestamp>& earliest() const noexcept;

    [[nodiscard]] const std::optional<foundation::Timestamp>& latest() const noexcept;

  private:
    std::optional<foundation::Timestamp> m_earliest;
    std::optional<foundation::Timestamp> m_latest;
};

} // namespace scope::investigation
