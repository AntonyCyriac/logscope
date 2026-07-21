/**
 * @file json_lines_parser.hpp
 * @brief Minimal JSON Lines (NDJSON) line parser.
 */

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "json_field_mapping.hpp"
#include "log_line_classifier.hpp"

namespace scope::analysis
{

/**
 * @brief Outcome of parsing a single JSON Lines record.
 */
enum class JsonLineParseOutcome
{
    Blank,
    Valid,
    Invalid
};

/**
 * @brief Parsed content from a single JSON Lines record.
 */
struct JsonLineParseResult
{
    /// Parse outcome for the line.
    JsonLineParseOutcome outcome{JsonLineParseOutcome::Invalid};

    /// Top-level object keys when @p outcome is Valid.
    std::vector<std::string> topLevelKeys;

    /// Raw level field value when a supported key is present.
    std::string levelValue;

    /// Raw timestamp field value when a supported key is present.
    std::string timestampValue;

    /// Raw message field value when a supported key is present.
    std::string messageValue;

    /// Raw correlation/trace field value when a supported key is present.
    std::string correlationValue;
};

/**
 * @brief Parses one JSON Lines record.
 */
class JsonLinesParser
{
  public:
    /**
     * @brief Parses a single line as a JSON object.
     *
     * @param line Line content without trailing newline.
     */
    [[nodiscard]] static JsonLineParseResult parse(std::string_view line) noexcept;

    /**
     * @brief Parses a single line using configurable field mapping.
     */
    [[nodiscard]] static JsonLineParseResult parse(std::string_view line,
                                                 const JsonFieldMapping& mapping) noexcept;
};

/**
 * @brief Maps a JSON level field value to a detected log level.
 *
 * @param fieldValue Value from level, severity, or nested log.level.
 */
[[nodiscard]] DetectedLogLevel detectLogLevelFromJsonField(std::string_view fieldValue) noexcept;

} // namespace scope::analysis
