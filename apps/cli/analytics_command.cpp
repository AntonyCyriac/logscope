/**
 * @file analytics_command.cpp
 * @brief Analytics subcommand implementation.
 */

#include "analytics_command.hpp"

#include "analytics_engine.hpp"
#include "analytics_output.hpp"
#include "analysis.hpp"
#include "cli_analysis_config.hpp"
#include "cli_config.hpp"
#include "extension.hpp"
#include "log_macros.hpp"
#include "source.hpp"

namespace scope::cli
{

void printAnalyticsUsage(std::ostream& output)
{
    output << "Usage: logscope analytics [--config <file>] [--format text|json] "
           << "[--log-format auto|plain|jsonl] [--profile <name>] [analytics options] <log-source>\n"
           << "\n"
           << "Analytics options:\n"
           << "  --bucket <seconds>      Timeline bucket size (default: auto)\n"
           << "  --top <n>               Top frequency/cluster results (default: 10)\n"
           << "\n"
           << "Log source may be a file path, a directory of .log files, or \"-\" for stdin.\n";
}

int runAnalyticsCommand(const AnalyticsOptions& options,
                        configuration::ConfigurationManager& configurationManager,
                        std::ostream& output,
                        std::ostream& errorOutput)
{
    if (options.showHelp)
    {
        printAnalyticsUsage(output);

        return 0;
    }

    if (options.format != OutputFormat::Text && options.format != OutputFormat::Json)
    {
        errorOutput << "Analytics supports only text or json output formats." << '\n';

        return 1;
    }

    if (!initializeConfiguration(options.configFile, configurationManager, errorOutput))
    {
        return 1;
    }

    scope::extension::ExtensionManager extensionManager = scope::extension::ExtensionManager::createWithBuiltIns();
    extensionManager.applyConfiguration(configurationManager.configuration());
    extensionManager.initializeEnabled();

    SCOPE_LOG_INFO("cli", "Running analytics on " + options.logFile.string());

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

    analytics::AnalyticsConfig analyticsConfig = options.analyticsConfig;

    if (configurationManager.configuration().has("analytics.bucket_seconds"))
    {
        const auto bucketValue = configurationManager.configuration().get("analytics.bucket_seconds");

        if (bucketValue)
        {
            try
            {
                analyticsConfig.bucketSeconds = std::stoll(*bucketValue);
            }
            catch (...)
            {
            }
        }
    }

    if (configurationManager.configuration().has("analytics.top_n"))
    {
        const auto topValue = configurationManager.configuration().get("analytics.top_n");

        if (topValue)
        {
            try
            {
                analyticsConfig.topN = static_cast<std::size_t>(std::stoull(*topValue));
            }
            catch (...)
            {
            }
        }
    }

    if (configurationManager.configuration().has("analytics.min_cluster_count"))
    {
        const auto minClusterValue = configurationManager.configuration().get("analytics.min_cluster_count");

        if (minClusterValue)
        {
            try
            {
                analyticsConfig.minClusterCount = std::stoull(*minClusterValue);
            }
            catch (...)
            {
            }
        }
    }

    const analytics::AnalyticsResult analytics = analytics::AnalyticsEngine{}.analyze(*modelResult, analyticsConfig);
    writeAnalyticsOutput(analytics, options.format, output);

    return 0;
}

} // namespace scope::cli
