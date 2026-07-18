/**
 * @file log_line_classifier.hpp
 * @brief Generic log line level detection.
 */

#pragma once

#include <string_view>

namespace scope::analysis
{

/**
 * @brief Detected log severity for a single line.
 */
enum class DetectedLogLevel
{
    Blank,
    Info,
    Warn,
    Error,
    Other
};

/**
 * @brief Classifies a log line using generic pattern matching.
 */
[[nodiscard]] DetectedLogLevel detectLogLevel(std::string_view line) noexcept;

} // namespace scope::analysis
