/**
 * @file analyze_command.cpp
 * @brief Analyze subcommand implementation.
 */

#include "analyze_command.hpp"

#include "cli_analysis_config.hpp"
#include "log_analyzer.hpp"
#include "cli_config.hpp"
#include "extension.hpp"
#include "log_macros.hpp"
#include "report_config.hpp"

namespace scope::cli
{

void printAnalyzeUsage(std::ostream& output)
{
    output << "Usage: logscope analyze [--config <file>] [--format text|json|csv|markdown|html|pdf] "
              "[--log-format auto|plain|jsonl] [--profile <name>] [--sections <list>] "
              "[--output <file>] <log-source>\n"
           << "\n"
           << "Options:\n"
           << "  --config <file>       Load configuration from a properties file\n"
           << "  --format <format>     Output format: text, json, csv, markdown, html, or pdf (default: text)\n"
           << "  --log-format <name>   Input format: auto, plain, or jsonl (default: auto)\n"
           << "  --profile <name>      Built-in format profile: generic-plain, generic-json\n"
           << "  --sections <list>     Comma-separated sections: executive, summary, levels, errors,\n"
           << "                        charts, metadata, formats, all (default: all)\n"
           << "  --output <file>       Write report to file instead of stdout (creates parent dirs)\n"
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
    const scope::analysis::AnalysisConfig analysisConfig = buildAnalysisConfig(options, configurationManager);

    if (!analyzer.analyze(options.logFile, reportOptions, analysisConfig, options.outputFile, output, errorOutput))
    {
        return 1;
    }

    return 0;
}

} // namespace scope::cli
