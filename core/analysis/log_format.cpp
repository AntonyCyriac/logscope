/**
 * @file log_format.cpp
 * @brief LogFormat parsing helpers.
 */

#include "log_format.hpp"

#include "foundation/string.hpp"

namespace scope::analysis
{

std::optional<LogFormat> parseLogFormat(const std::string_view name) noexcept
{
    const std::string normalized = foundation::toLower(name);

    if (normalized == "auto")
    {
        return LogFormat::Auto;
    }

    if (normalized == "plain" || normalized == "text" || normalized == "plaintext")
    {
        return LogFormat::PlainText;
    }

    if (normalized == "jsonl" || normalized == "ndjson" || normalized == "json-lines")
    {
        return LogFormat::JsonLines;
    }

    return std::nullopt;
}

} // namespace scope::analysis
