/**
 * @file format_parser.hpp
 * @brief Format parser contract for future dynamic extensions (M7).
 */

#pragma once

#include <string_view>

#include "json_field_mapping.hpp"
#include "json_lines_parser.hpp"
#include "log_format.hpp"

namespace scope::analysis
{

/**
 * @brief Context passed to format parsers during analysis.
 */
struct FormatParseContext
{
    LogFormat format{LogFormat::PlainText};
    JsonFieldMapping jsonFieldMapping;
};

/**
 * @brief Parses a single log line according to a format contract.
 */
class FormatParser
{
  public:
    virtual ~FormatParser() = default;

    [[nodiscard]] virtual JsonLineParseResult parseLine(std::string_view line) const noexcept = 0;
};

/**
 * @brief JSON Lines parser backed by the built-in hand-rolled implementation.
 */
class JsonLinesFormatParser final : public FormatParser
{
  public:
    explicit JsonLinesFormatParser(JsonFieldMapping mapping) noexcept;

    [[nodiscard]] JsonLineParseResult parseLine(std::string_view line) const noexcept override;

  private:
    JsonFieldMapping m_mapping;
};

} // namespace scope::analysis
