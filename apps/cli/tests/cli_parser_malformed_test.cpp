/**
 * @file cli_parser_malformed_test.cpp
 * @brief Malformed-input tests for CLI argument parsing.
 */

#include <gtest/gtest.h>

#include "cli_parser.hpp"

using scope::cli::CliCommand;
using scope::cli::parseCliArguments;

namespace
{

char* toArgv(std::string& value)
{
    return value.data();
}

} // namespace

TEST(CliParserMalformedTest, RejectsUnknownOption)
{
    std::string program = "logscope";
    std::string command = "analyze";
    std::string badFlag = "--not-a-real-flag";
    std::string logFile = "sample.log";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(badFlag), toArgv(logFile)};

    const auto parsed = parseCliArguments(4, argv);

    EXPECT_FALSE(parsed.has_value());
}

TEST(CliParserMalformedTest, RejectsMissingLogFileForAnalyze)
{
    std::string program = "logscope";
    std::string command = "analyze";
    char* argv[] = {toArgv(program), toArgv(command)};

    const auto parsed = parseCliArguments(2, argv);

    EXPECT_FALSE(parsed.has_value());
}

TEST(CliParserMalformedTest, RejectsInvalidFormatValue)
{
    std::string program = "logscope";
    std::string command = "analyze";
    std::string formatFlag = "--format";
    std::string formatValue = "not-a-format";
    std::string logFile = "sample.log";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(formatFlag), toArgv(formatValue), toArgv(logFile)};

    const auto parsed = parseCliArguments(5, argv);

    EXPECT_FALSE(parsed.has_value());
}

TEST(CliParserMalformedTest, RejectsSessionLoadWithoutFile)
{
    std::string program = "logscope";
    std::string command = "session";
    std::string subcommand = "load";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(subcommand)};

    const auto parsed = parseCliArguments(3, argv);

    EXPECT_FALSE(parsed.has_value());
}

TEST(CliParserMalformedTest, HandlesHelpFlagWithoutCrashing)
{
    std::string program = "logscope";
    std::string helpFlag = "--help";
    char* argv[] = {toArgv(program), toArgv(helpFlag)};

    const auto parsed = parseCliArguments(2, argv);

    ASSERT_TRUE(parsed.has_value());
    EXPECT_TRUE(parsed->showGlobalHelp);
}
