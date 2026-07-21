/**
 * @file format_parser.cpp
 * @brief Built-in format parser implementations.
 */

#include "format_parser.hpp"

namespace scope::analysis
{

JsonLinesFormatParser::JsonLinesFormatParser(const JsonFieldMapping mapping) noexcept : m_mapping(mapping)
{
}

JsonLineParseResult JsonLinesFormatParser::parseLine(const std::string_view line) const noexcept
{
    return JsonLinesParser::parse(line, m_mapping);
}

} // namespace scope::analysis
