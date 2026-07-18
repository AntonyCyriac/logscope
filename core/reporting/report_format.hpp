/**
 * @file report_format.hpp
 * @brief Supported report output formats.
 */

#pragma once

#include <optional>
#include <string_view>

namespace scope::reporting
{

/**
 * @brief Output format for generated reports.
 */
enum class ReportFormat
{
    Text,
    Json,
    Csv,
    Markdown
};

/**
 * @brief Parses a report format name.
 */
[[nodiscard]] std::optional<ReportFormat> parseReportFormat(std::string_view value) noexcept;

/**
 * @brief Returns the canonical format name.
 */
[[nodiscard]] std::string_view reportFormatName(ReportFormat format) noexcept;

} // namespace scope::reporting
