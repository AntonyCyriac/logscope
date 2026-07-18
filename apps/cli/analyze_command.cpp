/**
 * @file analyze_command.cpp
 * @brief Analyze subcommand implementation.
 */

#include "analyze_command.hpp"

#include "LogAnalyzer.hpp"
#include "cli_config.hpp"
#include "log_macros.hpp"

namespace scope::cli
{

void printAnalyzeUsage(std::ostream& output)
{
    output << "Usage: logscope analyze [--config <file>] [--format text|json] <log-file>\n"
           << "\n"
           << "Options:\n"
           << "  --config <file>   Load configuration from a properties file\n"
           << "  --format <format> Output format: text or json (default: text)\n"
           << "  --help, -h        Show this help message\n";
}

int runAnalyzeCommand(const AnalyzeOptions& options,
                      configuration::ConfigurationManager& configurationManager,
                      std::ostream& output,
                      std::ostream& errorOutput)
{
    if (options.showHelp)
    {
        printAnalyzeUsage(output);

        return 0;
    }

    if (!initializeConfiguration(options.configFile, configurationManager, errorOutput))
    {
        return 1;
    }

    SCOPE_LOG_INFO("cli", "Analyzing " + options.logFile.string());

    LogAnalyzer analyzer;

    if (!analyzer.analyze(options.logFile, options.format, output))
    {
        return 1;
    }

    return 0;
}

} // namespace scope::cli
