/**
 * @file config_validate_command.cpp
 * @brief Config validate subcommand implementation.
 */

#include "config_validate_command.hpp"

#include "search.hpp"
#include "cli_analysis_config.hpp"
#include "cli_config.hpp"
#include "search_history.hpp"

namespace scope::cli
{

void printConfigValidateUsage(std::ostream& output)
{
    output << "Usage: logscope config validate [--config <file>] [--require key1,key2,...]\n"
           << "\n"
           << "Options:\n"
           << "  --config <file>      Load configuration from a properties file\n"
           << "  --require <keys>     Comma-separated list of required configuration keys\n"
           << "  --help, -h           Show this help message\n";
}

int runConfigValidateCommand(const ConfigValidateOptions& options,
                             configuration::ConfigurationManager& configurationManager,
                             std::ostream& output,
                             std::ostream& errorOutput)
{
    if (options.showHelp)
    {
        printConfigValidateUsage(output);

        return 0;
    }

    if (!initializeConfiguration(options.configFile, configurationManager, errorOutput))
    {
        return 1;
    }

    const auto validationResult = configurationManager.validate(options.requiredKeys);

    if (!validationResult)
    {
        errorOutput << validationResult.error().message() << std::endl;

        return 1;
    }

    const auto analysisValidation = scope::analysis::validateAnalysisConfiguration(configurationManager.configuration());

    if (!analysisValidation)
    {
        errorOutput << analysisValidation.error().message() << std::endl;

        return 1;
    }

    const auto searchValidation = scope::search::validateSearchConfiguration(configurationManager.configuration());

    if (!searchValidation)
    {
        errorOutput << searchValidation.error().message() << std::endl;

        return 1;
    }

    output << "Configuration is valid." << std::endl;

    return 0;
}

} // namespace scope::cli
