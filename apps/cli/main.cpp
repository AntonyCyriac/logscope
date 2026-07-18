/**
 * @file main.cpp
 * @brief LogScope CLI entry point.
 */

#include <iostream>
#include <optional>
#include <string>

#include "LogAnalyzer.hpp"

#include "configuration_manager.hpp"
#include "foundation/path.hpp"
#include "runtime.hpp"

namespace
{

struct CliOptions
{
    scope::foundation::Path logFile;
    scope::foundation::Path configFile;
    bool showHelp = false;
};

void printUsage(std::ostream& output)
{
    output << "Usage: logscope [--config <file>] <log-file>\n"
           << "\n"
           << "Options:\n"
           << "  --config <file>  Load configuration from a properties file\n"
           << "  --help, -h       Show this help message\n";
}

std::optional<CliOptions> parseArguments(int argc, char* argv[])
{
    CliOptions options;

    for (int index = 1; index < argc; ++index)
    {
        const std::string argument = argv[index];

        if (argument == "--help" || argument == "-h")
        {
            options.showHelp = true;

            return options;
        }

        if (argument == "--config")
        {
            if (index + 1 >= argc)
            {
                return std::nullopt;
            }

            options.configFile = scope::foundation::Path(argv[++index]);

            continue;
        }

        if (!argument.empty() && argument.front() == '-')
        {
            return std::nullopt;
        }

        if (!options.logFile.string().empty())
        {
            return std::nullopt;
        }

        options.logFile = scope::foundation::Path(argument);
    }

    if (options.showHelp)
    {
        return options;
    }

    if (options.logFile.string().empty())
    {
        return std::nullopt;
    }

    return options;
}

bool initializeConfiguration(const scope::foundation::Path& configFilePath,
                           scope::configuration::ConfigurationManager& manager)
{
    if (configFilePath.string().empty())
    {
        manager.applyEnvironment();

        return scope::runtime::Diagnostics::instance().applyConfiguration(manager.configuration());
    }

    const auto loadResult = scope::configuration::ConfigurationManager::loadFromFile(configFilePath);

    if (!loadResult)
    {
        std::cerr << "Failed to load configuration: " << loadResult.error().message() << std::endl;

        return false;
    }

    manager = std::move(*loadResult);
    manager.applyEnvironment();

    if (!scope::runtime::Diagnostics::instance().applyConfiguration(manager.configuration()))
    {
        std::cerr << "Failed to apply configuration." << std::endl;

        return false;
    }

    return true;
}

} // namespace

int main(int argc, char* argv[])
{
    const auto options = parseArguments(argc, argv);

    if (!options)
    {
        printUsage(std::cerr);

        return 1;
    }

    if (options->showHelp)
    {
        printUsage(std::cout);

        return 0;
    }

    scope::configuration::ConfigurationManager configurationManager;

    if (!initializeConfiguration(options->configFile, configurationManager))
    {
        return 1;
    }

    SCOPE_LOG_INFO("cli", "Analyzing " + options->logFile.string());

    scope::cli::LogAnalyzer analyzer;

    if (!analyzer.analyze(options->logFile))
    {
        return 1;
    }

    return 0;
}
