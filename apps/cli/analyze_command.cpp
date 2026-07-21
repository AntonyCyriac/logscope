/**
 * @file analyze_command.cpp
 * @brief Analyze subcommand implementation.
 */

#include "analyze_command.hpp"

#include "log_analyzer.hpp"
#include "cli_config.hpp"
#include "extension.hpp"
#include "log_macros.hpp"
#include "report_config.hpp"

namespace scope::cli
{

void printAnalyzeUsage(std::ostream& output)
{
    output << "Usage: logscope analyze [--config <file>] [--format text|json|csv|markdown] "
              "[--log-format auto|plain|jsonl] [--sections <list>] <log-source>\n"
           << "\n"
           << "Options:\n"
           << "  --config <file>       Load configuration from a properties file\n"
           << "  --format <format>     Output format: text, json, csv, or markdown (default: text)\n"
           << "  --log-format <name>   Input format: auto, plain, or jsonl (default: auto)\n"
           << "  --sections <list>     Comma-separated sections: summary, levels, metadata, all\n"
           << "                        (default: all; config key: report.sections)\n"
           << "  --help, -h            Show this help message\n"
           << "\n"
           << "Log source may be a file path, a directory of .log files, or \"-\" for stdin.\n";
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

    scope::extension::ExtensionManager extensionManager = scope::extension::ExtensionManager::createWithBuiltIns();
    extensionManager.applyConfiguration(configurationManager.configuration());
    extensionManager.initializeEnabled();

    SCOPE_LOG_INFO("cli", "Analyzing " + options.logFile.string());

    LogAnalyzer analyzer;

    const reporting::ReportOptions reportOptions = buildReportOptions(options, configurationManager);

    if (!analyzer.analyze(options.logFile, reportOptions, options.logFormat, output, errorOutput))
    {
        return 1;
    }

    return 0;
}

} // namespace scope::cli
