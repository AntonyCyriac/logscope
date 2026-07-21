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

TEST(CliParserTest, ParsesAnalyzeLogFormat)
{
    std::string program = "logscope";
    std::string command = "analyze";
    std::string logFormatFlag = "--log-format";
    std::string logFormatValue = "plain";
    std::string logFile = "sample.log";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(logFormatFlag), toArgv(logFormatValue),
                    toArgv(logFile)};

    const auto parsed = parseCliArguments(5, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::Analyze, parsed->command);
    EXPECT_EQ(scope::analysis::LogFormat::PlainText, parsed->analyze.logFormat);
}

TEST(CliParserTest, RejectsInvalidLogFormat)
{
    std::string program = "logscope";
    std::string command = "analyze";
    std::string logFormatFlag = "--log-format";
    std::string logFormatValue = "xml";
    std::string logFile = "sample.log";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(logFormatFlag), toArgv(logFormatValue),
                    toArgv(logFile)};

    const auto parsed = parseCliArguments(5, argv);

    EXPECT_FALSE(parsed);
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

TEST(CliParserTest, ParsesStdinAnalyzeSubcommand)
{
    std::string program = "logscope";
    std::string command = "analyze";
    std::string stdinSource = "-";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(stdinSource)};

    const auto parsed = parseCliArguments(3, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::Analyze, parsed->command);
    EXPECT_EQ("-", parsed->analyze.logFile.string());
}

TEST(CliParserTest, ParsesAnalyzeSectionsOption)
{
    std::string program = "logscope";
    std::string command = "analyze";
    std::string sectionsFlag = "--sections";
    std::string sectionsValue = "summary,levels";
    std::string logFile = "sample.log";
    char* argv[] = {toArgv(program),     toArgv(command), toArgv(sectionsFlag),
                    toArgv(sectionsValue), toArgv(logFile)};

    const auto parsed = parseCliArguments(5, argv);

    ASSERT_TRUE(parsed);
    ASSERT_TRUE(parsed->analyze.sections);
    EXPECT_TRUE(parsed->analyze.sections->includes(scope::reporting::ReportSection::Summary));
    EXPECT_TRUE(parsed->analyze.sections->includes(scope::reporting::ReportSection::LevelBreakdown));
    EXPECT_FALSE(parsed->analyze.sections->includes(scope::reporting::ReportSection::SourceMetadata));
}

TEST(CliParserTest, ParsesExtensionsListSubcommand)
{
    std::string program = "logscope";
    std::string command = "extensions";
    std::string subcommand = "list";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(subcommand)};

    const auto parsed = parseCliArguments(3, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::ExtensionsList, parsed->command);
}

TEST(CliParserTest, ParsesExtensionsDescribeSubcommand)
{
    std::string program = "logscope";
    std::string command = "extensions";
    std::string subcommand = "describe";
    std::string extensionId = "analysis.log-levels";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(subcommand), toArgv(extensionId)};

    const auto parsed = parseCliArguments(4, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::ExtensionsDescribe, parsed->command);
    EXPECT_EQ("analysis.log-levels", parsed->extensionsDescribe.extensionId);
}

TEST(CliParserTest, ParsesSessionSaveSubcommand)
{
    std::string program = "logscope";
    std::string command = "session";
    std::string subcommand = "save";
    std::string sessionFile = "sample.logscope-session";
    std::string logFile = "sample.log";
    std::string minErrorsFlag = "--min-errors";
    std::string minErrorsValue = "1";
    char* argv[] = {toArgv(program),     toArgv(command),      toArgv(subcommand), toArgv(sessionFile),
                    toArgv(logFile),     toArgv(minErrorsFlag),  toArgv(minErrorsValue)};

    const auto parsed = parseCliArguments(7, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::SessionSave, parsed->command);
    EXPECT_EQ("sample.logscope-session", parsed->sessionSave.sessionFile.string());
    EXPECT_EQ("sample.log", parsed->sessionSave.logFile.string());
    EXPECT_EQ(1U, parsed->sessionSave.minErrors);
}

TEST(CliParserTest, ParsesSessionLoadSubcommand)
{
    std::string program = "logscope";
    std::string command = "session";
    std::string subcommand = "load";
    std::string sessionFile = "sample.logscope-session";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(subcommand), toArgv(sessionFile)};

    const auto parsed = parseCliArguments(4, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::SessionLoad, parsed->command);
    EXPECT_EQ("sample.logscope-session", parsed->sessionLoad.sessionFile.string());
}

TEST(CliParserTest, ParsesInvestigateSubcommand)
{
    std::string program = "logscope";
    std::string command = "investigate";
    std::string searchFlag = "--search";
    std::string searchValue = "refused";
    std::string logFile = "sample.log";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(searchFlag), toArgv(searchValue), toArgv(logFile)};

    const auto parsed = parseCliArguments(5, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::Investigate, parsed->command);
    EXPECT_EQ("sample.log", parsed->investigate.logFile.string());
    EXPECT_EQ("refused", parsed->investigate.criteria.contentSearch);
}

TEST(CliParserTest, ParsesSessionSaveContentSearch)
{
    std::string program = "logscope";
    std::string command = "session";
    std::string subcommand = "save";
    std::string sessionFile = "sample.logscope-session";
    std::string logFile = "sample.log";
    std::string contentSearchFlag = "--content-search";
    std::string contentSearchValue = "timeout";
    char* argv[] = {toArgv(program),     toArgv(command),           toArgv(subcommand), toArgv(sessionFile),
                    toArgv(logFile),     toArgv(contentSearchFlag), toArgv(contentSearchValue)};

    const auto parsed = parseCliArguments(7, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::SessionSave, parsed->command);
    EXPECT_EQ("timeout", parsed->sessionSave.contentCriteria.contentSearch);
}

TEST(CliParserTest, RejectsInvalidOption)
{
    std::string program = "logscope";
    std::string invalidFlag = "--unknown";
    char* argv[] = {toArgv(program), toArgv(invalidFlag)};

    EXPECT_FALSE(parseCliArguments(2, argv));
}
