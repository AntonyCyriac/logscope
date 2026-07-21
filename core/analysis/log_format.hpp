/**
 * @file log_format.hpp
 * @brief Identifies supported log input formats.
 */

#pragma once

#include <optional>
#include <string_view>

namespace scope::analysis
{

/**
 * @brief Supported log input formats for detection and analysis.
 *
 * @note Auto is a CLI/engine hint meaning "detect from content". It is never
 * stored as a detected format on AnalysisModel.
 */
enum class LogFormat
{
    Auto,
    PlainText,
    JsonLines,
    Unknown
};

/**
 * @brief Returns the stable short name for a log format (plain, jsonl, auto, unknown).
 */
[[nodiscard]] constexpr std::string_view logFormatName(const LogFormat format) noexcept
{
    switch (format)
    {
    case LogFormat::Auto:
        return "auto";
    case LogFormat::PlainText:
        return "plain";
    case LogFormat::JsonLines:
        return "jsonl";
    case LogFormat::Unknown:
        return "unknown";
    }

    return "unknown";
}

/**
 * @brief Parses a log format name (auto, plain, jsonl).
 */
[[nodiscard]] std::optional<LogFormat> parseLogFormat(std::string_view name) noexcept;

} // namespace scope::analysis
