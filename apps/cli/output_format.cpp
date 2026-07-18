/**
 * @file output_format.cpp
 * @brief OutputFormat implementation.
 */

#include "output_format.hpp"

#include "foundation/string.hpp"

namespace scope::cli
{

std::optional<OutputFormat> parseOutputFormat(const std::string_view value) noexcept
{
    if (foundation::toLower(value) == "text")
    {
        return OutputFormat::Text;
    }

    if (foundation::toLower(value) == "json")
    {
        return OutputFormat::Json;
    }

    return std::nullopt;
}

std::string_view outputFormatName(const OutputFormat format) noexcept
{
    switch (format)
    {
    case OutputFormat::Text:
        return "text";
    case OutputFormat::Json:
        return "json";
    }

    return "text";
}

} // namespace scope::cli
