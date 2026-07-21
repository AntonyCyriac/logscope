/**
 * @file correlation_id_extractor.hpp
 * @brief Extracts correlation and trace identifiers from log lines.
 */

#pragma once

#include <string>
#include <string_view>

namespace scope::analysis
{

/**
 * @brief Extracts a correlation or trace identifier from a log line.
 *
 * @param line Raw log line content.
 * @param jsonCorrelationValue Value from a JSON correlation field when present.
 */
[[nodiscard]] std::string extractCorrelationId(std::string_view line,
                                             std::string_view jsonCorrelationValue) noexcept;

} // namespace scope::analysis
