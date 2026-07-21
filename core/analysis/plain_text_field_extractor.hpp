/**
 * @file plain_text_field_extractor.hpp
 * @brief Plain-text timestamp and message field extraction.
 */

#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "foundation/result.hpp"
#include "foundation/timestamp.hpp"

namespace scope::analysis
{

/**
 * @brief Extracted fields from a plain-text log line.
 */
struct PlainTextFields
{
    /// Parsed leading timestamp when present.
    std::optional<foundation::Timestamp> timestamp;

    /// Message excerpt after the timestamp and optional level token.
    std::string messageExcerpt;
};

/**
 * @brief Extracts timestamp and message fields from a plain-text log line.
 */
class PlainTextFieldExtractor
{
  public:
    /**
     * @brief Extracts fields from a single line.
     *
     * Supports ISO-8601 (`YYYY-MM-DDTHH:MM:SS`) and common space-separated
     * prefixes (`YYYY-MM-DD HH:MM:SS`).
     */
    [[nodiscard]] static PlainTextFields extract(std::string_view line) noexcept;
};

/**
 * @brief Parses a log timestamp string into a Timestamp.
 */
[[nodiscard]] foundation::Result<foundation::Timestamp> parseLogTimestamp(std::string_view value) noexcept;

} // namespace scope::analysis
