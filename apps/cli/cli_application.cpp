/**
 * @file cli_application.cpp
 * @brief CLI command dispatcher implementation.
 */

#include "cli_application.hpp"

#include "analyze_command.hpp"
#include "config_validate_command.hpp"

namespace scope::cli
{

void CliApplication::printUsage(std::ostream& output)
{
    output << "Usage: logscope <command> [options]\n"
           << "       logscope [--config <file>] <log-source>\n"
           << "\n"
           << "Commands:\n"
           << "  analyze            Analyze a log file, directory, or stdin\n"
           << "  config validate    Validate configuration\n"
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
    case CliCommand::ConfigValidate:
        return runConfigValidateCommand(parsed.configValidate, configurationManager, output, errorOutput);
    case CliCommand::Help:
        printUsage(output);

        return 0;
    }

    printUsage(errorOutput);

    return 1;
}

} // namespace scope::cli
