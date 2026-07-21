/**
 * @file field_filter.hpp
 * @brief Filters indexed lines by level, message, or JSON key presence.
 */

#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "line_index.hpp"
#include "log_line_classifier.hpp"

namespace scope::investigation
{

/**
 * @brief Progressive per-line field filter for indexed log lines.
 */
class FieldFilter
{
  public:
    [[nodiscard]] static FieldFilter any() noexcept;

    [[nodiscard]] FieldFilter withLevel(analysis::DetectedLogLevel level) const noexcept;

    [[nodiscard]] FieldFilter withMessageContains(std::string_view substring) const noexcept;

    [[nodiscard]] FieldFilter withRequiredJsonKey(std::string_view key) const noexcept;

    [[nodiscard]] bool isActive() const noexcept;

    [[nodiscard]] bool matches(const analysis::IndexedLine& line) const noexcept;

    [[nodiscard]] const std::optional<analysis::DetectedLogLevel>& level() const noexcept;

    [[nodiscard]] const std::string& messageContains() const noexcept;

    [[nodiscard]] const std::string& requiredJsonKey() const noexcept;

  private:
    std::optional<analysis::DetectedLogLevel> m_level;
    std::string m_messageContains;
    std::string m_requiredJsonKey;
};

} // namespace scope::investigation
