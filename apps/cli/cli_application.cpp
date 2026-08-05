/**
 * @file cli_application.cpp
 * @brief CLI command dispatcher implementation.
 */

#include "cli_application.hpp"

#include "agent_command.hpp"
#include "analyze_command.hpp"
#include "analytics_command.hpp"
#include "config_validate_command.hpp"
#include "extensions_command.hpp"
#include "investigate_command.hpp"
#include "investigation_command.hpp"
#include "query_command.hpp"
#include "search_command.hpp"
#include "session_command.hpp"

namespace scope::cli
{

void CliApplication::printUsage(std::ostream& output)
{
    output << "Usage: logscope <command> [options]\n"
           << "       logscope [--config <file>] <log-source>\n"
           << "\n"
           << "Commands:\n"
           << "  analyze            Analyze a log file, directory, or stdin\n"
           << "  investigate        Search and filter indexed log content\n"
           << "  search             Alias for investigate focused on search queries\n"
           << "  query              Run field-aware filter DSL queries\n"
           << "  analytics            Run frequency, clustering, and timeline analytics\n"
           << "  config validate    Validate configuration\n"
           << "  extensions list    List available extensions\n"
           << "  extensions describe Describe an extension\n"
           << "  session save       Save investigation context to a session file\n"
           << "  session load       Restore a session and reproduce its report\n"
           << "  session list       List saved session files in a directory\n"
           << "  investigation create  Create a portable investigation container\n"
           << "  investigation add     Add an artifact to an investigation (log, pstack, core)\n"
           << "  investigation add-note Add a note artifact to an investigation\n"
           << "  investigation list    List investigations in a root directory\n"
           << "  investigation show    Show investigation metadata and artifacts\n"
           << "  investigation open    Print resolved log path for an investigation\n"
           << "  investigation timeline Show chronological investigation timeline\n"
           << "  agent investigate  AI-assisted log investigation\n"
           << "  help [command]     Show help for a command\n"
           << "\n"
           << "Global options:\n"
           << "  --help, -h         Show this help message\n";
}

int CliApplication::run(const ParsedCli& parsed,
                        configuration::ConfigurationManager& configurationManager,
                        std::ostream& output,
                        std::ostream& errorOutput) const
{
    if (parsed.showGlobalHelp)
    {
        printUsage(output);

        return 0;
    }

    switch (parsed.command)
    {
    case CliCommand::Analyze:
        return runAnalyzeCommand(parsed.analyze, configurationManager, output, errorOutput);
    case CliCommand::Investigate:
        return runInvestigateCommand(parsed.investigate, configurationManager, output, errorOutput);
    case CliCommand::Search:
        return runSearchCommand(parsed.search, configurationManager, output, errorOutput);
    case CliCommand::Query:
        return runQueryCommand(parsed.query, configurationManager, output, errorOutput);
    case CliCommand::Analytics:
        return runAnalyticsCommand(parsed.analytics, configurationManager, output, errorOutput);
    case CliCommand::ConfigValidate:
        return runConfigValidateCommand(parsed.configValidate, configurationManager, output, errorOutput);
    case CliCommand::ExtensionsList:
        return runExtensionsListCommand(parsed.extensionsList, configurationManager, output, errorOutput);
    case CliCommand::ExtensionsDescribe:
        return runExtensionsDescribeCommand(parsed.extensionsDescribe, configurationManager, output, errorOutput);
    case CliCommand::SessionSave:
        return runSessionSaveCommand(parsed.sessionSave, configurationManager, output, errorOutput);
    case CliCommand::SessionLoad:
        return runSessionLoadCommand(parsed.sessionLoad, output, errorOutput);
    case CliCommand::SessionList:
        return runSessionListCommand(parsed.sessionList, output, errorOutput);
    case CliCommand::InvestigationCreate:
        return runInvestigationCreateCommand(parsed.investigationCreate, output, errorOutput);
    case CliCommand::InvestigationAdd:
        return runInvestigationAddCommand(parsed.investigationAdd, output, errorOutput);
    case CliCommand::InvestigationAddNote:
        return runInvestigationAddNoteCommand(parsed.investigationAddNote, output, errorOutput);
    case CliCommand::InvestigationList:
        return runInvestigationListCommand(parsed.investigationList, output, errorOutput);
    case CliCommand::InvestigationShow:
        return runInvestigationShowCommand(parsed.investigationShow, output, errorOutput);
    case CliCommand::InvestigationOpen:
        return runInvestigationOpenCommand(parsed.investigationOpen, output, errorOutput);
    case CliCommand::InvestigationTimeline:
        return runInvestigationTimelineCommand(parsed.investigationTimeline, output, errorOutput);
    case CliCommand::AgentHelp:
        printAgentUsage(output);

        return 0;
    case CliCommand::AgentInvestigate:
        return runAgentInvestigateCommand(parsed.agentInvestigate, configurationManager, output, errorOutput);
    case CliCommand::Help:
        printUsage(output);

        return 0;
    }

    printUsage(errorOutput);

    return 1;
}

} // namespace scope::cli
