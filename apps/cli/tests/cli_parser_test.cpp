/**
 * @file cli_parser_test.cpp
 * @brief Unit tests for CLI argument parsing.
 */

#include <gtest/gtest.h>

#include "cli_parser.hpp"
#include "search_query.hpp"

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
    EXPECT_FALSE(parsed->analyze.format.has_value());
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

TEST(CliParserTest, ParsesAnalyzeProfileOption)
{
    std::string program = "logscope";
    std::string command = "analyze";
    std::string profileFlag = "--profile";
    std::string profileValue = "generic-json";
    std::string logFile = "sample.jsonl";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(profileFlag), toArgv(profileValue),
                    toArgv(logFile)};

    const auto parsed = parseCliArguments(5, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::Analyze, parsed->command);
    EXPECT_EQ("generic-json", parsed->analyze.profile);
}

TEST(CliParserTest, ParsesInvestigateProfileOption)
{
    std::string program = "logscope";
    std::string command = "investigate";
    std::string profileFlag = "--profile";
    std::string profileValue = "generic-plain";
    std::string logFile = "sample.log";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(profileFlag), toArgv(profileValue),
                    toArgv(logFile)};

    const auto parsed = parseCliArguments(5, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::Investigate, parsed->command);
    EXPECT_EQ("generic-plain", parsed->investigate.profile);
}

TEST(CliParserTest, ParsesInvestigateQueryOption)
{
    std::string program = "logscope";
    std::string command = "investigate";
    std::string queryFlag = "--query";
    std::string queryValue = "error AND refused";
    std::string logFile = "sample.log";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(queryFlag), toArgv(queryValue), toArgv(logFile)};

    const auto parsed = parseCliArguments(5, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::Investigate, parsed->command);
    EXPECT_EQ("error AND refused", parsed->investigate.criteria.booleanQuery);
}

TEST(CliParserTest, ParsesSearchSubcommandWithRegexFlag)
{
    std::string program = "logscope";
    std::string command = "search";
    std::string searchFlag = "--search";
    std::string searchValue = "error.*";
    std::string regexFlag = "--regex";
    std::string logFile = "sample.log";
    char* argv[] = {toArgv(program),     toArgv(command), toArgv(searchFlag), toArgv(searchValue),
                    toArgv(regexFlag),   toArgv(logFile)};

    const auto parsed = parseCliArguments(6, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::Search, parsed->command);
    EXPECT_EQ("error.*", parsed->search.criteria.contentSearch);
    EXPECT_EQ(scope::search::SearchMode::Regex, parsed->search.criteria.searchMode);
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

TEST(CliParserTest, ParsesInvestigationCreateOptions)
{
    std::string program = "logscope";
    std::string command = "investigation";
    std::string subcommand = "create";
    std::string nameFlag = "--name";
    std::string nameValue = "incident-alpha";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(subcommand), toArgv(nameFlag), toArgv(nameValue)};

    const auto parsed = parseCliArguments(5, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::InvestigationCreate, parsed->command);
    EXPECT_EQ("incident-alpha", parsed->investigationCreate.name);
}

TEST(CliParserTest, ParsesInvestigationCreateDescriptionAndDir)
{
    std::string program = "logscope";
    std::string command = "investigation";
    std::string subcommand = "create";
    std::string nameFlag = "--name";
    std::string nameValue = "incident-beta";
    std::string descriptionFlag = "--description";
    std::string descriptionValue = "customer outage";
    std::string dirFlag = "--dir";
    std::string dirValue = "/tmp/workspaces";
    char* argv[] = {toArgv(program),       toArgv(command),       toArgv(subcommand), toArgv(nameFlag),
                    toArgv(nameValue),       toArgv(descriptionFlag), toArgv(descriptionValue), toArgv(dirFlag),
                    toArgv(dirValue)};

    const auto parsed = parseCliArguments(9, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::InvestigationCreate, parsed->command);
    EXPECT_EQ("incident-beta", parsed->investigationCreate.name);
    EXPECT_EQ("customer outage", parsed->investigationCreate.description);
    EXPECT_EQ("/tmp/workspaces", parsed->investigationCreate.rootDirectory.string());
}

TEST(CliParserTest, ParsesInvestigationAddNoteOptions)
{
    std::string program = "logscope";
    std::string command = "investigation";
    std::string subcommand = "add-note";
    std::string investigationId = "00000000-0000-4000-8000-000000000002";
    std::string titleFlag = "--title";
    std::string titleValue = "timeline";
    std::string bodyFlag = "--body";
    std::string bodyValue = "first failure at 09:15";
    char* argv[] = {toArgv(program),     toArgv(command),   toArgv(subcommand), toArgv(investigationId),
                    toArgv(titleFlag),   toArgv(titleValue), toArgv(bodyFlag),   toArgv(bodyValue)};

    const auto parsed = parseCliArguments(8, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::InvestigationAddNote, parsed->command);
    EXPECT_EQ(investigationId, parsed->investigationAddNote.investigationId);
    EXPECT_EQ("timeline", parsed->investigationAddNote.title);
    EXPECT_EQ("first failure at 09:15", parsed->investigationAddNote.body);
}

TEST(CliParserTest, ParsesInvestigationListWithDir)
{
    std::string program = "logscope";
    std::string command = "investigation";
    std::string subcommand = "list";
    std::string dirFlag = "--dir";
    std::string dirValue = "/var/logscope/workspaces";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(subcommand), toArgv(dirFlag), toArgv(dirValue)};

    const auto parsed = parseCliArguments(5, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::InvestigationList, parsed->command);
    EXPECT_EQ("/var/logscope/workspaces", parsed->investigationList.rootDirectory.string());
}

TEST(CliParserTest, ParsesInvestigationShowWithDir)
{
    std::string program = "logscope";
    std::string command = "investigation";
    std::string subcommand = "show";
    std::string investigationId = "00000000-0000-4000-8000-000000000003";
    std::string dirFlag = "--dir";
    std::string dirValue = "/var/logscope/workspaces";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(subcommand), toArgv(investigationId),
                    toArgv(dirFlag),   toArgv(dirValue)};

    const auto parsed = parseCliArguments(6, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::InvestigationShow, parsed->command);
    EXPECT_EQ(investigationId, parsed->investigationShow.investigationId);
    EXPECT_EQ("/var/logscope/workspaces", parsed->investigationShow.rootDirectory.string());
}

TEST(CliParserTest, ParsesInvestigationAddTypeAndRole)
{
    std::string program = "logscope";
    std::string command = "investigation";
    std::string subcommand = "add";
    std::string investigationId = "00000000-0000-4000-8000-000000000001";
    std::string source = "dump.core";
    std::string typeFlag = "--type";
    std::string typeValue = "core";
    std::string roleFlag = "--role";
    std::string roleValue = "kernel";
    char* argv[] = {toArgv(program),     toArgv(command),   toArgv(subcommand), toArgv(investigationId),
                    toArgv(source),      toArgv(typeFlag),  toArgv(typeValue),  toArgv(roleFlag),
                    toArgv(roleValue)};

    const auto parsed = parseCliArguments(9, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::InvestigationAdd, parsed->command);
    EXPECT_EQ("core", parsed->investigationAdd.artifactType);
    EXPECT_EQ("kernel", parsed->investigationAdd.role);
}

TEST(CliParserTest, ParsesInvestigationOpenArtifact)
{
    std::string program = "logscope";
    std::string command = "investigation";
    std::string subcommand = "open";
    std::string investigationId = "00000000-0000-4000-8000-000000000001";
    std::string artifactFlag = "--artifact";
    std::string artifactId = "00000000-0000-4000-8000-000000000099";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(subcommand), toArgv(investigationId),
                    toArgv(artifactFlag), toArgv(artifactId)};

    const auto parsed = parseCliArguments(6, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::InvestigationOpen, parsed->command);
    EXPECT_EQ(artifactId, parsed->investigationOpen.artifactId);
}

TEST(CliParserTest, ParsesInvestigationAddPstackType)
{
    std::string program = "logscope";
    std::string command = "investigation";
    std::string subcommand = "add";
    std::string investigationId = "00000000-0000-4000-8000-000000000001";
    std::string source = "threads.pstack";
    std::string typeFlag = "--type";
    std::string typeValue = "pstack";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(subcommand), toArgv(investigationId),
                    toArgv(source), toArgv(typeFlag), toArgv(typeValue)};

    const auto parsed = parseCliArguments(7, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::InvestigationAdd, parsed->command);
    EXPECT_EQ("pstack", parsed->investigationAdd.artifactType);
    EXPECT_EQ("threads.pstack", parsed->investigationAdd.logFile.string());
}

TEST(CliParserTest, ParsesInvestigationAddDisplayNameAndDir)
{
    std::string program = "logscope";
    std::string command = "investigation";
    std::string subcommand = "add";
    std::string investigationId = "00000000-0000-4000-8000-000000000001";
    std::string source = "syslog.log";
    std::string displayFlag = "--display-name";
    std::string displayValue = "syslog";
    std::string dirFlag = "--dir";
    std::string dirValue = "/tmp/workspaces";
    char* argv[] = {toArgv(program),     toArgv(command),   toArgv(subcommand), toArgv(investigationId),
                    toArgv(source),      toArgv(displayFlag), toArgv(displayValue), toArgv(dirFlag),
                    toArgv(dirValue)};

    const auto parsed = parseCliArguments(9, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ("syslog", parsed->investigationAdd.displayName);
    EXPECT_EQ("/tmp/workspaces", parsed->investigationAdd.rootDirectory.string());
}

TEST(CliParserTest, ParsesInvestigationOpenDir)
{
    std::string program = "logscope";
    std::string command = "investigation";
    std::string subcommand = "open";
    std::string investigationId = "00000000-0000-4000-8000-000000000001";
    std::string dirFlag = "--dir";
    std::string dirValue = "/tmp/workspaces";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(subcommand), toArgv(investigationId),
                    toArgv(dirFlag), toArgv(dirValue)};

    const auto parsed = parseCliArguments(6, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::InvestigationOpen, parsed->command);
    EXPECT_EQ("/tmp/workspaces", parsed->investigationOpen.rootDirectory.string());
    EXPECT_TRUE(parsed->investigationOpen.artifactId.empty());
}

TEST(CliParserTest, RejectsInvestigationAddWithUnknownOption)
{
    std::string program = "logscope";
    std::string command = "investigation";
    std::string subcommand = "add";
    std::string investigationId = "00000000-0000-4000-8000-000000000001";
    std::string source = "app.log";
    std::string unknownFlag = "--unknown";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(subcommand), toArgv(investigationId),
                    toArgv(source), toArgv(unknownFlag)};

    EXPECT_FALSE(parseCliArguments(6, argv));
}

TEST(CliParserTest, RejectsInvestigationAddMissingSource)
{
    std::string program = "logscope";
    std::string command = "investigation";
    std::string subcommand = "add";
    std::string investigationId = "00000000-0000-4000-8000-000000000001";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(subcommand), toArgv(investigationId)};

    EXPECT_FALSE(parseCliArguments(4, argv));
}

TEST(CliParserTest, ParsesInvestigationTimelineOptions)
{
    std::string program = "logscope";
    std::string command = "investigation";
    std::string subcommand = "timeline";
    std::string investigationId = "00000000-0000-4000-8000-000000000004";
    std::string formatFlag = "--format";
    std::string formatValue = "json";
    std::string limitFlag = "--limit";
    std::string limitValue = "25";
    std::string orderFlag = "--order";
    std::string orderValue = "desc";
    std::string dirFlag = "--dir";
    std::string dirValue = "/tmp/workspaces";
    char* argv[] = {toArgv(program),     toArgv(command),   toArgv(subcommand), toArgv(investigationId),
                    toArgv(formatFlag),  toArgv(formatValue), toArgv(limitFlag), toArgv(limitValue),
                    toArgv(orderFlag),   toArgv(orderValue), toArgv(dirFlag), toArgv(dirValue)};

    const auto parsed = parseCliArguments(12, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::InvestigationTimeline, parsed->command);
    EXPECT_EQ(investigationId, parsed->investigationTimeline.investigationId);
    EXPECT_EQ(scope::cli::InvestigationTimelineFormat::Json, parsed->investigationTimeline.format);
    ASSERT_TRUE(parsed->investigationTimeline.limit.has_value());
    EXPECT_EQ(25U, *parsed->investigationTimeline.limit);
    EXPECT_EQ(scope::workspace::TimelineSortOrder::Descending, parsed->investigationTimeline.order);
    EXPECT_EQ("/tmp/workspaces", parsed->investigationTimeline.rootDirectory.string());
}

TEST(CliParserTest, RejectsInvestigationTimelineInvalidFormat)
{
    std::string program = "logscope";
    std::string command = "investigation";
    std::string subcommand = "timeline";
    std::string investigationId = "00000000-0000-4000-8000-000000000004";
    std::string formatFlag = "--format";
    std::string formatValue = "xml";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(subcommand), toArgv(investigationId),
                    toArgv(formatFlag), toArgv(formatValue)};

    EXPECT_FALSE(parseCliArguments(6, argv));
}

TEST(CliParserTest, RejectsInvalidOption)
{
    std::string program = "logscope";
    std::string invalidFlag = "--unknown";
    char* argv[] = {toArgv(program), toArgv(invalidFlag)};

    EXPECT_FALSE(parseCliArguments(2, argv));
}

TEST(CliParserTest, ParsesAgentHelp)
{
    std::string program = "logscope";
    std::string command = "agent";
    std::string helpFlag = "--help";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(helpFlag)};

    const auto parsed = parseCliArguments(3, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::AgentHelp, parsed->command);
}

TEST(CliParserTest, ParsesAgentInvestigateOptions)
{
    std::string program = "logscope";
    std::string command = "agent";
    std::string subcommand = "investigate";
    std::string askFlag = "--ask";
    std::string askValue = "errors";
    std::string summarizeFlag = "--summarize";
    std::string hintsFlag = "--hints";
    std::string logFile = "sample.log";
    char* argv[] = {toArgv(program),     toArgv(command),      toArgv(subcommand), toArgv(askFlag),
                    toArgv(askValue),    toArgv(summarizeFlag), toArgv(hintsFlag),  toArgv(logFile)};

    const auto parsed = parseCliArguments(8, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::AgentInvestigate, parsed->command);
    EXPECT_EQ("errors", parsed->agentInvestigate.askQuery);
    EXPECT_TRUE(parsed->agentInvestigate.summarize);
    EXPECT_TRUE(parsed->agentInvestigate.hints);
    EXPECT_EQ("sample.log", parsed->agentInvestigate.investigate.logFile.string());
}

TEST(CliParserTest, ParsesInvestigationLinksListOptions)
{
    std::string program = "logscope";
    std::string command = "investigation";
    std::string subcommand = "links";
    std::string verb = "list";
    std::string investigationId = "00000000-0000-4000-8000-000000000005";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(subcommand), toArgv(verb), toArgv(investigationId)};

    const auto parsed = parseCliArguments(5, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::InvestigationLinks, parsed->command);
    EXPECT_EQ(investigationId, parsed->investigationLinks.investigationId);
}

TEST(CliParserTest, ParsesHelpInvestigationLinks)
{
    std::string program = "logscope";
    std::string command = "help";
    std::string subcommand = "investigation";
    std::string verb = "links";
    char* argv[] = {toArgv(program), toArgv(command), toArgv(subcommand), toArgv(verb)};

    const auto parsed = parseCliArguments(4, argv);

    ASSERT_TRUE(parsed);
    EXPECT_EQ(CliCommand::InvestigationLinks, parsed->command);
    EXPECT_TRUE(parsed->investigationLinks.showHelp);
}
