/**
 * @file report_string_utils.hpp
 * @brief Shared escaping helpers for report formatters.
 */

#pragma once

#include <string>
#include <string_view>

namespace scope::reporting
{

[[nodiscard]] std::string escapeJsonString(std::string_view value);

[[nodiscard]] std::string escapeCsvField(std::string_view value);

[[nodiscard]] std::string escapeHtml(std::string_view value);

} // namespace scope::reporting
