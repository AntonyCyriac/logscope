/**
 * @file parser_registry_test.cpp
 */

#include <gtest/gtest.h>

#include "format_parser.hpp"
#include "json_lines_parser.hpp"
#include "parser_registry.hpp"

using scope::analysis::FormatParser;
using scope::analysis::JsonLineParseOutcome;
using scope::analysis::ParserRegistry;

namespace
{

class EchoFormatParser final : public FormatParser
{
  public:
    [[nodiscard]] scope::analysis::JsonLineParseResult parseLine(std::string_view line) const noexcept override
    {
        scope::analysis::JsonLineParseResult result;

        if (line.empty())
        {
            result.outcome = JsonLineParseOutcome::Blank;

            return result;
        }

        result.outcome = JsonLineParseOutcome::Valid;
        result.messageValue = std::string(line);

        return result;
    }
};

} // namespace

TEST(ParserRegistryTest, RegistersAndFindsParser)
{
    ParserRegistry::instance().clear();

    ParserRegistry::instance().registerParser("echo", []() { return std::make_unique<EchoFormatParser>(); });

    const FormatParser* parser = ParserRegistry::instance().findParser("echo");

    ASSERT_NE(parser, nullptr);

    const auto parsed = parser->parseLine("hello");

    ASSERT_EQ(JsonLineParseOutcome::Valid, parsed.outcome);
    EXPECT_EQ("hello", parsed.messageValue);
}
