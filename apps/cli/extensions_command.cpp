/**
 * @file extensions_command.cpp
 * @brief Extensions CLI subcommand implementation.
 */

#include "extensions_command.hpp"

#include <iomanip>

#include "cli_config.hpp"
#include "extension.hpp"

namespace scope::cli
{

namespace
{

std::string extensionStatusName(const scope::extension::ExtensionStatus status)
{
    switch (status)
    {
    case scope::extension::ExtensionStatus::Ready:
        return "ready";
    case scope::extension::ExtensionStatus::Disabled:
        return "disabled";
    case scope::extension::ExtensionStatus::InitializationFailed:
        return "failed";
    }

    return "unknown";
}

bool prepareConfiguration(const foundation::Path& configFile,
                          configuration::ConfigurationManager& configurationManager,
                          std::ostream& errorOutput)
{
    if (configFile.string().empty())
    {
        configurationManager.applyEnvironment();

        return true;
    }

    return initializeConfiguration(configFile, configurationManager, errorOutput);
}

scope::extension::ExtensionManager createConfiguredExtensionManager(
    const configuration::ConfigurationManager& configurationManager)
{
    scope::extension::ExtensionManager manager = scope::extension::ExtensionManager::createWithBuiltIns();
    manager.applyConfiguration(configurationManager.configuration());
    manager.initializeEnabled();

    return manager;
}

} // namespace

void printExtensionsListUsage(std::ostream& output)
{
    output << "Usage: logscope extensions list [--config <file>]\n"
           << "\n"
           << "Options:\n"
           << "  --config <file>   Load configuration from a properties file\n"
           << "  --help, -h        Show this help message\n";
}

void printExtensionsDescribeUsage(std::ostream& output)
{
    output << "Usage: logscope extensions describe [--config <file>] <extension-id>\n"
           << "\n"
           << "Options:\n"
           << "  --config <file>   Load configuration from a properties file\n"
           << "  --help, -h        Show this help message\n";
}

int runExtensionsListCommand(const ExtensionsListOptions& options,
                             configuration::ConfigurationManager& configurationManager,
                             std::ostream& output,
                             std::ostream& errorOutput)
{
    if (options.showHelp)
    {
        printExtensionsListUsage(output);

        return 0;
    }

    if (!prepareConfiguration(options.configFile, configurationManager, errorOutput))
    {
        return 1;
    }

    const scope::extension::ExtensionManager manager = createConfiguredExtensionManager(configurationManager);

    output << std::left << std::setw(28) << "ID" << std::setw(10) << "VERSION" << std::setw(10) << "ENABLED"
           << "STATUS" << '\n';

    for (const scope::extension::ExtensionInfo& info : manager.listExtensions())
    {
        output << std::left << std::setw(28) << info.id << std::setw(10) << info.version << std::setw(10)
               << (info.enabled ? "yes" : "no") << extensionStatusName(info.status) << '\n';
    }

    return 0;
}

int runExtensionsDescribeCommand(const ExtensionsDescribeOptions& options,
                                 configuration::ConfigurationManager& configurationManager,
                                 std::ostream& output,
                                 std::ostream& errorOutput)
{
    if (options.showHelp)
    {
        printExtensionsDescribeUsage(output);

        return 0;
    }

    if (!prepareConfiguration(options.configFile, configurationManager, errorOutput))
    {
        return 1;
    }

    const scope::extension::ExtensionManager manager = createConfiguredExtensionManager(configurationManager);

    const auto infoResult = manager.describe(options.extensionId);

    if (!infoResult)
    {
        errorOutput << infoResult.error().message() << std::endl;

        return 1;
    }

    output << "ID          : " << infoResult->id << '\n'
           << "Version     : " << infoResult->version << '\n'
           << "Enabled     : " << (infoResult->enabled ? "yes" : "no") << '\n'
           << "Status      : " << extensionStatusName(infoResult->status) << '\n'
           << "Description : " << infoResult->description << '\n';

    return 0;
}

} // namespace scope::cli
