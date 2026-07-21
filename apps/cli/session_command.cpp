/**
 * @file session_command.cpp
 * @brief Session CLI subcommand implementation.
 */

#include "session_command.hpp"

#include "analysis.hpp"
#include "cli_analysis_config.hpp"
#include "cli_config.hpp"
#include "extension.hpp"
#include "investigation.hpp"
#include "investigation_output.hpp"
#include "log_macros.hpp"
#include "report_config.hpp"
#include "report_output.hpp"
#include "source.hpp"
#include "workspace.hpp"

namespace scope::cli
{

namespace
{

investigation::LineCountFilter buildLineFilter(const SessionSaveOptions& options)
{
    investigation::LineCountFilter filter = investigation::LineCountFilter::any().withMinimum(options.minLines);

    if (options.maxLines)
    {
        filter = filter.withMaximum(*options.maxLines);
    }

    return filter;
}

investigation::LogLevelFilter buildLevelFilter(const SessionSaveOptions& options)
{
    return investigation::LogLevelFilter::any()
        .withMinimumErrors(options.minErrors)
        .withMinimumWarnings(options.minWarnings);
}

} // namespace

void printSessionSaveUsage(std::ostream& output)
{
    output << "Usage: logscope session save <session-file> <log-source> [options]\n"
           << "\n"
           << "Options:\n"
           << "  --config <file>       Load configuration from a properties file\n"
           << "  --format <format>     Report format: text, json, csv, or markdown\n"
           << "  --sections <list>     Report sections: summary, levels, metadata, all\n"
           << "  --min-errors <n>      Investigation filter: minimum error lines\n"
           << "  --min-warnings <n>    Investigation filter: minimum warning lines\n"
           << "  --min-lines <n>       Investigation filter: minimum total lines\n"
           << "  --max-lines <n>       Investigation filter: maximum total lines\n"
           << "  --search <query>      Investigation search query for source path\n"
           << "  --content-search <q>  Search indexed log line content\n"
           << "  --time-from <ts>      Earliest timestamp filter (ISO-like)\n"
           << "  --time-to <ts>        Latest timestamp filter (ISO-like)\n"
           << "  --level <name>        Per-line level filter: error, warning, info, other\n"
           << "  --message <text>      Message/content substring filter\n"
           << "  --json-key <key>      Require JSON top-level key on matching lines\n"
           << "  --help, -h            Show this help message\n";
}

void printSessionLoadUsage(std::ostream& output)
{
    output << "Usage: logscope session load <session-file>\n"
           << "\n"
           << "Loads a saved session and reproduces the report without re-analyzing the log source.\n"
           << "\n"
           << "Options:\n"
           << "  --help, -h            Show this help message\n";
}

void printSessionListUsage(std::ostream& output)
{
    output << "Usage: logscope session list [directory]\n"
           << "\n"
           << "Lists .logscope-session files in the given directory (default: current directory).\n"
           << "\n"
           << "Options:\n"
           << "  --help, -h            Show this help message\n";
}

int runSessionSaveCommand(const SessionSaveOptions& options,
                          configuration::ConfigurationManager& configurationManager,
                          std::ostream& output,
                          std::ostream& errorOutput)
{
    if (options.showHelp)
    {
        printSessionSaveUsage(output);

        return 0;
    }

    if (!initializeConfiguration(options.configFile, configurationManager, errorOutput))
    {
        return 1;
    }

    scope::extension::ExtensionManager extensionManager = scope::extension::ExtensionManager::createWithBuiltIns();
    extensionManager.applyConfiguration(configurationManager.configuration());
    extensionManager.initializeEnabled();

    scope::source::SourceManager sourceManager;

    auto datasetResult = sourceManager.open(options.logFile);

    if (!datasetResult)
    {
        SCOPE_LOG_ERROR("cli", datasetResult.error().message());

        errorOutput << datasetResult.error().message() << std::endl;

        return 1;
    }

    const scope::analysis::AnalysisConfig analysisConfig = buildAnalysisConfig(options, configurationManager);
    const auto modelResult = scope::analysis::AnalysisEngine{}.analyze(*datasetResult, analysisConfig);

    if (!modelResult)
    {
        SCOPE_LOG_ERROR("cli", modelResult.error().message());

        errorOutput << modelResult.error().message() << std::endl;

        return 1;
    }

    AnalyzeOptions analyzeOptions;
    analyzeOptions.configFile = options.configFile;
    analyzeOptions.format = options.format;
    analyzeOptions.sections = options.sections;

    const reporting::ReportOptions reportOptions = buildReportOptions(analyzeOptions, configurationManager);

    const scope::workspace::InvestigationSession session = scope::workspace::InvestigationSession::fromAnalysis(
        *modelResult, buildLineFilter(options), buildLevelFilter(options), options.searchQuery,
        options.contentCriteria, reportOptions, options.configFile);

    const scope::workspace::SessionStore store;

    const auto saveResult = store.save(session, options.sessionFile);

    if (!saveResult)
    {
        errorOutput << saveResult.error().message() << std::endl;

        return 1;
    }

    output << "Session saved to " << options.sessionFile.string() << std::endl;

    return 0;
}

int runSessionLoadCommand(const SessionLoadOptions& options, std::ostream& output, std::ostream& errorOutput)
{
    if (options.showHelp)
    {
        printSessionLoadUsage(output);

        return 0;
    }

    const scope::workspace::SessionStore store;

    const auto sessionResult = store.load(options.sessionFile);

    if (!sessionResult)
    {
        errorOutput << sessionResult.error().message() << std::endl;

        return 1;
    }

    const scope::workspace::InvestigationSession& session = *sessionResult;
    analysis::AnalysisModel model = session.analysisModel();

    if (session.contentCriteria().isActive())
    {
        scope::source::SourceManager sourceManager;

        if (auto datasetResult = sourceManager.open(session.sourcePath()))
        {
            const scope::analysis::AnalysisConfig analysisConfig =
                scope::analysis::AnalysisConfig::defaults();

            if (auto refreshedModel = scope::analysis::AnalysisEngine{}.analyze(*datasetResult, analysisConfig))
            {
                model = *refreshedModel;
            }
        }
    }

    scope::investigation::InvestigationEngine investigationEngine;

    const bool matchesFilters = investigationEngine.matches(model, session.lineFilter()) &&
                                investigationEngine.matches(model, session.levelFilter()) &&
                                (session.searchQuery().empty() ||
                                 investigationEngine.searchSource(model, session.searchQuery()));

    output << "Session ID  : " << session.sessionId().toString() << '\n'
           << "Source      : " << session.sourcePath().string() << '\n'
           << "Matches     : " << (matchesFilters ? "yes" : "no") << '\n';

    if (session.contentCriteria().isActive())
    {
        const scope::investigation::InvestigationResult investigationResult =
            investigationEngine.investigate(model, session.contentCriteria());

        output << '\n' << formatInvestigationOutput(investigationResult, OutputFormat::Text) << '\n';
    }

    output << '\n' << formatAnalysisOutput(model, session.reportOptions()) << std::endl;

    return 0;
}

int runSessionListCommand(const SessionListOptions& options, std::ostream& output, std::ostream& errorOutput)
{
    if (options.showHelp)
    {
        printSessionListUsage(output);

        return 0;
    }

    const scope::workspace::SessionStore store;

    const auto sessionsResult = store.list(options.directory);

    if (!sessionsResult)
    {
        errorOutput << sessionsResult.error().message() << std::endl;

        return 1;
    }

    if (sessionsResult->empty())
    {
        output << "No session files found." << std::endl;

        return 0;
    }

    for (const foundation::Path& sessionFile : *sessionsResult)
    {
        output << sessionFile.string() << '\n';
    }

    return 0;
}

} // namespace scope::cli
