/**
 * @file query_command.cpp
 * @brief Query subcommand implementation.
 */

#include "query_command.hpp"

#include "analysis.hpp"
#include "cli_analysis_config.hpp"
#include "cli_config.hpp"
#include "extension.hpp"
#include "foundation/string.hpp"
#include "investigation.hpp"
#include "investigation_output.hpp"
#include "log_macros.hpp"
#include "source.hpp"

namespace scope::cli
{

void printQueryUsage(std::ostream& output)
{
    output << "Usage: logscope query [--config <file>] [--format text|json] "
              "[--log-format auto|plain|jsonl] [--profile <name>] --filter <dsl> <log-source>\n"
           << "\n"
           << "Query options:\n"
           << "  --filter <dsl>          Field-aware filter expression (required)\n"
           << "  --search <query>        Optional text search over indexed content\n"
           << "  --query <expr>          Optional boolean text search expression\n"
           << "\n"
           << "Filter DSL examples:\n"
           << "  level == ERROR\n"
           << "  contains(message, \"timeout\")\n"
           << "  level == ERROR AND contains(message, \"refused\")\n"
           << "\n"
           << "Log source may be a file path, a directory of .log files, or \"-\" for stdin.\n";
}

int runQueryCommand(const InvestigateOptions& options,
                    configuration::ConfigurationManager& configurationManager,
                    std::ostream& output,
                    std::ostream& errorOutput)
{
    if (options.showHelp)
    {
        printQueryUsage(output);

        return 0;
    }

    if (foundation::isBlank(options.criteria.filterExpression) && !options.criteria.filterQuery.has_value())
    {
        errorOutput << "Query command requires --filter <dsl>." << '\n';

        return 1;
    }

    if (!initializeConfiguration(options.configFile, configurationManager, errorOutput))
    {
        return 1;
    }

    scope::extension::ExtensionManager extensionManager = scope::extension::ExtensionManager::createWithBuiltIns();
    extensionManager.applyConfiguration(configurationManager.configuration());
    extensionManager.initializeEnabled();

    SCOPE_LOG_INFO("cli", "Running query on " + options.logFile.string());

    scope::source::SourceManager sourceManager;

    auto datasetResult = sourceManager.open(options.logFile);

    if (!datasetResult)
    {
        errorOutput << datasetResult.error().message() << '\n';

        return 1;
    }

    const scope::analysis::AnalysisConfig analysisConfig = buildAnalysisConfig(options, configurationManager);
    const auto modelResult = scope::analysis::AnalysisEngine{}.analyze(*datasetResult, analysisConfig);

    if (!modelResult)
    {
        errorOutput << modelResult.error().message() << '\n';

        return 1;
    }

    const auto searchResult = options.criteria.resolvedSearchQuery();

    if (!searchResult)
    {
        errorOutput << searchResult.error().message() << '\n';

        return 1;
    }

    const auto filterResult = options.criteria.resolvedFilterQuery();

    if (!filterResult)
    {
        errorOutput << filterResult.error().message() << '\n';

        return 1;
    }

    scope::investigation::InvestigationEngine investigationEngine;
    const scope::investigation::InvestigationResult result =
        investigationEngine.investigate(*modelResult, options.criteria);

    output << formatInvestigationOutput(result, options.format) << std::endl;

    return 0;
}

} // namespace scope::cli
