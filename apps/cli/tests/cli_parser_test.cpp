/**
 * @file cli_parser_test.cpp
 * @brief Unit tests for CLI argument parsing.
 */

#include <gtest/gtest.h>

#include "cli_parser.hpp"

using scope::cli::CliCommand;
using scope::cli::OutputFormat;
using scope::cli::parseCliArguments;

namespace
{

char* toArgv(std::string& value)
{
    return value.data();
}

} // namespace

TEST(CliParserTest, ParsesAnalyzeSubcommand)
{
    std::string program = "logscope";
    std::string command = "analyze";
    std::string formatFlag = "--format";
    std::string formatValue = "json";
    std::string logFile = "sample.log";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(formatFlag), toArgv(formatValue), toArgv(logFile)};

    const auto parsed = parseCliArguments(5, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::Analyze, parsed->command);
    EXPECT_EQ("sample.log", parsed->analyze.logFile.string());
    EXPECT_EQ(OutputFormat::Json, parsed->analyze.format);
}

TEST(CliParserTest, ParsesLegacyAnalyzeInvocation)
{
    std::string program = "logscope";
    std::string configFlag = "--config";
    std::string configFile = "logscope.properties";
    std::string logFile = "sample.log";
    char* argv[] = {toArgv(program), toArgv(configFlag), toArgv(configFile), toArgv(logFile)};

    const auto parsed = parseCliArguments(4, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::Analyze, parsed->command);
    EXPECT_EQ("logscope.properties", parsed->analyze.configFile.string());
    EXPECT_EQ("sample.log", parsed->analyze.logFile.string());
    EXPECT_EQ(OutputFormat::Text, parsed->analyze.format);
}

TEST(CliParserTest, ParsesConfigValidateSubcommand)
{
    std::string program = "logscope";
    std::string configCommand = "config";
    std::string validateCommand = "validate";
    std::string requireFlag = "--require";
    std::string requiredKeys = "log.level,app.name";
    char* argv[] = {toArgv(program),       toArgv(configCommand), toArgv(validateCommand),
                    toArgv(requireFlag),   toArgv(requiredKeys)};

    const auto parsed = parseCliArguments(5, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::ConfigValidate, parsed->command);
    ASSERT_EQ(2U, parsed->configValidate.requiredKeys.size());
    EXPECT_EQ("log.level", parsed->configValidate.requiredKeys[0]);
    EXPECT_EQ("app.name", parsed->configValidate.requiredKeys[1]);
}

TEST(CliParserTest, ParsesGlobalHelp)
{
    std::string program = "logscope";
    std::string helpFlag = "--help";
    char* argv[] = {toArgv(program), toArgv(helpFlag)};

    const auto parsed = parseCliArguments(2, argv);

    ASSERT_TRUE(parsed);
    EXPECT_TRUE(parsed->showGlobalHelp);
}

TEST(CliParserTest, RejectsInvalidOption)
{
    std::string program = "logscope";
    std::string invalidFlag = "--unknown";
    char* argv[] = {toArgv(program), toArgv(invalidFlag)};

    EXPECT_FALSE(parseCliArguments(2, argv));
}
