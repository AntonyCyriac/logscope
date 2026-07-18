/**
 * @file output_format.hpp
 * @brief Supported CLI output formats.
 */

#pragma once

#include <optional>
#include <string_view>

#include "report_format.hpp"

namespace scope::cli
{

/**
 * @brief Output format for analysis results.
 */
enum class OutputFormat
{
    Text,
    Json,
    Csv,
    Markdown
};

/**
 * @brief Parses an output format name.
 */
[[nodiscard]] std::optional<OutputFormat> parseOutputFormat(std::string_view value) noexcept;

/**
 * @brief Returns the canonical format name.
 */
[[nodiscard]] std::string_view outputFormatName(OutputFormat format) noexcept;

/**
 * @brief Converts a CLI output format to a reporting format.
 */
[[nodiscard]] reporting::ReportFormat toReportFormat(OutputFormat format) noexcept;

} // namespace scope::cli
