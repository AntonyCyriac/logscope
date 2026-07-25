/**
 * @file parser_registry.hpp
 * @brief Registry for plugin format parsers (M12).
 */

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "format_parser.hpp"

namespace scope::analysis
{

using FormatParserFactory = std::function<std::unique_ptr<FormatParser>()>;

/**
 * @brief Registers and resolves plugin-provided format parsers.
 */
class ParserRegistry
{
  public:
    [[nodiscard]] static ParserRegistry& instance();

    void registerParser(const std::string& formatId, FormatParserFactory factory);

    [[nodiscard]] const FormatParser* findParser(const std::string& formatId) const;

    [[nodiscard]] std::vector<std::string> registeredFormatIds() const;

    void clear();

  private:
    ParserRegistry() = default;

    struct ParserEntry
    {
        std::string formatId;
        FormatParserFactory factory;
        std::unique_ptr<FormatParser> cachedInstance;
    };

    mutable std::vector<ParserEntry> m_parsers;
};

} // namespace scope::analysis
