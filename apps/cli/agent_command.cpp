/**
 * @file agent_command.cpp
 * @brief AI agent subcommand implementation (M13).
 */

#include "agent_command.hpp"

#include "ai_anomaly_hint_formatter.hpp"
#include "ai_config.hpp"
#include "ai_investigation_assistant.hpp"
#include "ai_summary_formatter.hpp"
#include "analysis.hpp"
#include "analytics_engine.hpp"
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

void printAgentUsage(std::ostream& output)
{
    output << "Usage: logscope agent <subcommand> [options]\n"
           << "\n"
           << "Subcommands:\n"
           << "  investigate        AI-assisted log investigation\n"
           << "\n"
           << "Use 'logscope agent investigate --help' for investigation options.\n";
}

void printAgentInvestigateUsage(std::ostream& output)
{
    output << "Usage: logscope agent investigate [--config <file>] [--format text|json] "
              "[--log-format auto|plain|jsonl] [--profile <name>] [investigation options] "
              "[--ask <query>] [--summarize] [--hints] <log-source>\n"
           << "\n"
           << "AI options:\n"
           << "  --ask <query>          Translate natural language to a filter and investigate\n"
           << "  --summarize            Include an AI investigation summary (requires ai.enabled=true)\n"
           << "  --hints                Include AI anomaly hints from analytics (requires ai.enabled=true)\n"
           << "  --stats                Print parse timing and resource usage stats\n"
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
           << "\n"
           << "Log source may be a file path, a directory of .log files, or \"-\" for stdin.\n";
}

int runAgentInvestigateCommand(const AgentInvestigateOptions& options,
                               configuration::ConfigurationManager& configurationManager,
                               std::ostream& output,
                               std::ostream& errorOutput)
{
    if (options.investigate.showHelp)
    {
        printAgentInvestigateUsage(output);

        return 0;
    }

    if (!initializeConfiguration(options.investigate.configFile, configurationManager, errorOutput))
    {
        return 1;
    }

    const scope::ai::AiConfig aiConfig =
        scope::ai::resolveAiConfig(configurationManager.configuration());
    const scope::ai::AiInvestigationAssistant assistant(aiConfig);

    scope::plugin::PluginLoadStats pluginStats;

    const scope::extension::ExtensionManager extensionManager =
        scope::plugin::createConfiguredExtensionManager(configurationManager.configuration(), &pluginStats);

    SCOPE_LOG_INFO("cli", "Agent investigating " + options.investigate.logFile.string());

    scope::source::SourceManager sourceManager;

    auto datasetResult = sourceManager.open(options.investigate.logFile);

    if (!datasetResult)
    {
        errorOutput << datasetResult.error().message() << '\n';

        return 1;
    }

    const scope::analysis::AnalysisConfig analysisConfig =
        buildAnalysisConfig(options.investigate, configurationManager);
    scope::analysis::AnalysisStats analysisStats;
    const auto modelResult = scope::analysis::AnalysisEngine{}.analyze(
        *datasetResult,
        analysisConfig,
        options.investigate.showStats ? &analysisStats : nullptr);

    if (!modelResult)
    {
        errorOutput << modelResult.error().message() << '\n';

        return 1;
    }

    scope::investigation::InvestigationEngine investigationEngine;
    scope::investigation::InvestigationCriteria criteria = options.investigate.criteria;
    scope::investigation::applyInvestigationConfiguration(criteria,
                                                          configurationManager.configuration());

    if (!options.askQuery.empty())
    {
        const auto filterResult = assistant.translateNaturalLanguageQuery(options.askQuery);

        if (!filterResult)
        {
            errorOutput << filterResult.error().message() << '\n';

            return 1;
        }

        criteria.filterExpression = *filterResult;
    }

    scope::investigation::InvestigationResult result;

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

    output << formatInvestigationOutput(result, options.investigate.format) << std::endl;

    const scope::investigation::InvestigationView view = investigationEngine.inspect(*modelResult);

    if (options.summarize)
    {
        if (!aiConfig.enabled)
        {
            SCOPE_LOG_INFO("cli", "Skipping AI summary because ai.enabled=false");
        }
        else
        {
            const auto summary = assistant.summarizeInvestigation(view, result);

            if (summary)
            {
                output << "\n========== AI SUMMARY ==========\n";
                output << scope::ai::formatAiSummary(*summary) << std::endl;
            }
            else
            {
                errorOutput << "AI summary skipped: " << summary.error().message() << '\n';
            }
        }
    }

    if (options.hints)
    {
        if (!aiConfig.enabled)
        {
            SCOPE_LOG_INFO("cli", "Skipping AI hints because ai.enabled=false");
        }
        else
        {
            const scope::analytics::AnalyticsEngine analyticsEngine;
            const auto analytics = analyticsEngine.analyze(*modelResult);
            const auto hints = assistant.suggestAnomalyHints(analytics);

            if (hints)
            {
                const std::string formatted = scope::ai::formatAiAnomalyHints(*hints);

                if (!formatted.empty())
                {
                    output << "\n========== AI ANOMALY HINTS ==========\n";
                    output << formatted << std::endl;
                }
            }
            else
            {
                errorOutput << "AI hints skipped: " << hints.error().message() << '\n';
            }
        }
    }

    if (options.investigate.showStats)
    {
        printRunStats(analysisStats, pluginStats, errorOutput);
    }

    (void)extensionManager;

    return 0;
}

} // namespace scope::cli
