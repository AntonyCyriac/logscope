/**
 * @file output_format.hpp
 * @brief Supported CLI output formats.
 */

#pragma once

#include <optional>
#include <string_view>

namespace scope::cli
{

/**
 * @brief Output format for analysis results.
 */
enum class OutputFormat
{
    Text,
    Json
};

/**
 * @brief Parses an output format name.
 */
[[nodiscard]] std::optional<OutputFormat> parseOutputFormat(std::string_view value) noexcept;

/**
 * @brief Returns the canonical format name.
 */
[[nodiscard]] std::string_view outputFormatName(OutputFormat format) noexcept;

} // namespace scope::cli
