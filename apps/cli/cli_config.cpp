/**
 * @file cli_config.cpp
 * @brief CLI configuration initialization helpers.
 */

#include "cli_config.hpp"

#include "runtime.hpp"

namespace scope::cli
{

bool initializeConfiguration(const foundation::Path& configFilePath,
                             configuration::ConfigurationManager& manager,
                             std::ostream& errorOutput)
{
    if (configFilePath.string().empty())
    {
        manager.applyEnvironment();

        return scope::runtime::Diagnostics::instance().applyConfiguration(manager.configuration());
    }

    const auto loadResult = configuration::ConfigurationManager::loadFromFile(configFilePath);

    if (!loadResult)
    {
        errorOutput << "Failed to load configuration: " << loadResult.error().message() << std::endl;

        return false;
    }

    manager = std::move(*loadResult);
    manager.applyEnvironment();

    if (!scope::runtime::Diagnostics::instance().applyConfiguration(manager.configuration()))
    {
        errorOutput << "Failed to apply configuration." << std::endl;

        return false;
    }

    return true;
}

} // namespace scope::cli
