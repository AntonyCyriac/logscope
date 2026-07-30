/**
 * @file investigate_command.cpp
 * @brief Investigate subcommand implementation.
 */

#include "investigate_command.hpp"

#include "analysis.hpp"
#include "cli_analysis_config.hpp"
#include "cli_config.hpp"
#include "cli_extension_runtime.hpp"
#include "investigation.hpp"
#include "investigation_output.hpp"
#include "log_macros.hpp"
#include "plugin_runtime.hpp"
#include "source.hpp"
#include "stats_output.hpp"

namespace scope::cli
{

void printInvestigateUsage(std::ostream& output)
{
    output << "Usage: logscope investigate [--config <file>] [--format text|json] "
              "[--log-format auto|plain|jsonl] [--profile <name>] [investigation options] <log-source>\n"
           << "\n"
           << "Investigation options:\n"
           << "  --search <query>        Search indexed log line content\n"
           << "  --query <expr>          Boolean search expression (AND, OR, NOT, quotes)\n"
           << "  --filter <dsl>          Field-aware filter expression\n"
           << "  --regex                 Treat --search value as a regular expression\n"
           << "  --case-sensitive        Disable default case-insensitive matching\n"
           << "  --time-from <timestamp> Earliest timestamp (ISO-like)\n"
           << "  --time-to <timestamp>   Latest timestamp (ISO-like)\n"
           << "  --level <name>          Filter by line level: error, warning, info, other\n"
           << "  --message <text>        Filter by message/content substring\n"
           << "  --json-key <key>        Require a JSON top-level key on matching lines\n"
           << "  --persist-index         Persist indexed lines to SQLite\n"
           << "  --reuse-index           Reuse an existing index when the source fingerprint matches\n"
           << "  --index-path <file>     Explicit SQLite index file path\n"
           << "  --stats                 Print parse timing and resource usage stats\n"
           << "\n"
           << "Log source may be a file path, a directory of .log files, or \"-\" for stdin.\n";
}

int runInvestigateCommand(const InvestigateOptions& options,
                          configuration::ConfigurationManager& configurationManager,
                          std::ostream& output,
                          std::ostream& errorOutput)
{
    if (options.showHelp)
    {
        printInvestigateUsage(output);

        return 0;
    }

    if (!initializeConfiguration(options.configFile, configurationManager, errorOutput))
    {
        return 1;
    }

    scope::plugin::PluginLoadStats pluginStats;
    const scope::extension::ExtensionManager extensionManager =
        scope::plugin::createConfiguredExtensionManager(configurationManager.configuration(), &pluginStats);

    SCOPE_LOG_INFO("cli", "Investigating " + options.logFile.string());

    scope::source::SourceManager sourceManager;

    auto datasetResult = sourceManager.open(options.logFile);

    if (!datasetResult)
    {
        errorOutput << datasetResult.error().message() << '\n';

        return 1;
    }

    const scope::analysis::AnalysisConfig analysisConfig = buildAnalysisConfig(options, configurationManager);
    scope::analysis::AnalysisStats analysisStats;
    const auto modelResult =
        scope::analysis::AnalysisEngine{}.analyze(*datasetResult,
                                                  analysisConfig,
                                                  options.showStats ? &analysisStats : nullptr);

    if (!modelResult)
    {
        errorOutput << modelResult.error().message() << '\n';

        return 1;
    }

    scope::investigation::InvestigationEngine investigationEngine;
    scope::investigation::InvestigationResult result;

    scope::investigation::InvestigationCriteria criteria = options.criteria;
    scope::investigation::applyInvestigationConfiguration(criteria,
                                                          configurationManager.configuration());

    if (criteria.isActive())
    {
        const auto queryResult = criteria.resolvedSearchQuery();

        if (!queryResult)
        {
            errorOutput << queryResult.error().message() << '\n';

            return 1;
        }

        const auto filterResult = criteria.resolvedFilterQuery();

        if (!filterResult)
        {
            errorOutput << filterResult.error().message() << '\n';

            return 1;
        }

        result = investigationEngine.investigate(*modelResult, criteria);
    }
    else
    {
        result.correlations = investigationEngine.findCorrelations(*modelResult);

        if (modelResult->lineIndex().has_value())
        {
            result.indexedLineCount = modelResult->lineIndex()->indexedLineCount();
            result.truncatedLineCount = modelResult->lineIndex()->truncatedLineCount();
        }
    }

    output << formatInvestigationOutput(result, options.format) << std::endl;

    if (options.showStats)
    {
        printRunStats(analysisStats, pluginStats, errorOutput);
    }

    return 0;
}

} // namespace scope::cli
