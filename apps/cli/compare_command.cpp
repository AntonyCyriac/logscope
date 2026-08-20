/**
 * @file compare_command.cpp
 * @brief Compare subcommand implementation.
 */

#include "compare_command.hpp"

#include "analysis.hpp"
#include "cli_analysis_config.hpp"
#include "cli_config.hpp"
#include "compare_output.hpp"
#include "discovery_census.hpp"
#include "foundation/error.hpp"
#include "log_macros.hpp"
#include "source.hpp"

namespace scope::cli
{

namespace
{

[[nodiscard]] int exitCodeForError(const foundation::Error& error) noexcept
{
    if (error.code() == foundation::ErrorCode::Indeterminate)
    {
        return 2;
    }

    return 1;
}

[[nodiscard]] std::optional<scope::compare::IncomparableReason>
incomparableReasonForOpenError(const foundation::Error& error, const foundation::Path& path) noexcept
{
    if (error.code() == foundation::ErrorCode::Indeterminate)
    {
        return scope::compare::IncomparableReason::BaselineIndeterminate;
    }

    if (error.code() == foundation::ErrorCode::InvalidArgument && scope::source::isArchivePath(path))
    {
        return scope::compare::IncomparableReason::BaselineUnsupported;
    }

    return std::nullopt;
}

struct LoadedRun
{
    std::optional<scope::analysis::AnalysisModel> model;
    std::optional<scope::compare::IncomparableReason> incomparableReason;
    bool hardError{false};
    foundation::Error error;
};

[[nodiscard]] LoadedRun loadRun(const foundation::Path& path, const scope::analysis::AnalysisConfig& analysisConfig,
                                const scope::source::OpenOptions& openOptions, const bool isBaseline)
{
    LoadedRun loaded;

    if (scope::source::isArchivePath(path))
    {
        loaded.incomparableReason =
            isBaseline ? scope::compare::IncomparableReason::BaselineUnsupported
                       : scope::compare::IncomparableReason::CandidateUnsupported;

        return loaded;
    }

    scope::source::SourceManager sourceManager;
    auto datasetResult = sourceManager.open(path, openOptions);

    if (!datasetResult)
    {
        loaded.error = datasetResult.error();

        if (const auto reason = incomparableReasonForOpenError(datasetResult.error(), path))
        {
            loaded.incomparableReason =
                isBaseline ? *reason
                           : (*reason == scope::compare::IncomparableReason::BaselineIndeterminate
                                  ? scope::compare::IncomparableReason::CandidateIndeterminate
                                  : scope::compare::IncomparableReason::CandidateUnsupported);
        }
        else
        {
            loaded.hardError = true;
        }

        return loaded;
    }

    const auto modelResult = scope::analysis::AnalysisEngine{}.analyze(*datasetResult, analysisConfig);

    if (!modelResult)
    {
        loaded.error = modelResult.error();
        loaded.hardError = true;

        return loaded;
    }

    loaded.model = std::move(*modelResult);

    return loaded;
}

} // namespace

void printCompareUsage(std::ostream& output)
{
    output << "Usage: logscope compare [--config <file>] [--format text|json] [--no-recursive] "
              "<baseline> <candidate>\n"
           << "\n"
           << "Options:\n"
           << "  --config <file>       Load configuration from a properties file\n"
           << "  --format <format>     Output format: text or json (default: text)\n"
           << "  --no-recursive        Do not recurse when opening directory sources\n"
           << "  --help, -h            Show this help message\n"
           << "\n"
           << "Compares two log sources (baseline vs candidate) and reports signature differences.\n";
}

int runCompareCommand(const CompareOptions& options,
                      configuration::ConfigurationManager& configurationManager,
                      std::ostream& output,
                      std::ostream& errorOutput)
{
    if (options.showHelp)
    {
        printCompareUsage(output);

        return 0;
    }

    if (!initializeConfiguration(options.configFile, configurationManager, errorOutput))
    {
        return 1;
    }

    const scope::analysis::AnalysisConfig analysisConfig = buildAnalysisConfig(options, configurationManager);

    scope::source::OpenOptions openOptions;
    openOptions.discovery.recursive = !options.noRecursive;

    SCOPE_LOG_INFO("cli",
                   "Comparing baseline=" + options.baselinePath.string() + " candidate=" +
                       options.candidatePath.string());

    const LoadedRun baselineRun = loadRun(options.baselinePath, analysisConfig, openOptions, true);

    if (baselineRun.hardError)
    {
        errorOutput << baselineRun.error.message() << '\n';

        return exitCodeForError(baselineRun.error);
    }

    if (baselineRun.incomparableReason.has_value())
    {
        writeCompareOutput(scope::compare::incomparableResult(*baselineRun.incomparableReason), options.format, output);

        return 2;
    }

    const LoadedRun candidateRun = loadRun(options.candidatePath, analysisConfig, openOptions, false);

    if (candidateRun.hardError)
    {
        errorOutput << candidateRun.error.message() << '\n';

        return exitCodeForError(candidateRun.error);
    }

    if (candidateRun.incomparableReason.has_value())
    {
        writeCompareOutput(scope::compare::incomparableResult(*candidateRun.incomparableReason), options.format, output);

        return 2;
    }

    const scope::compare::ComparisonResult comparison =
        scope::compare::compareModels(*baselineRun.model, *candidateRun.model);

    writeCompareOutput(comparison, options.format, output);

    if (!comparison.comparable)
    {
        return 2;
    }

    return 0;
}

} // namespace scope::cli
