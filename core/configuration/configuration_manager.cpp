/**
 * @file configuration_manager.cpp
 * @brief ConfigurationManager implementation.
 */

#include "configuration_manager.hpp"

#include <fstream>
#include <sstream>
#include <string>

#include "foundation/error.hpp"
#include "foundation/filesystem.hpp"
#include "foundation/string.hpp"

#if defined(_WIN32)
#include <stdlib.h>
extern "C" char** _environ;
#else
extern char** environ;
#endif

namespace scope::configuration
{

namespace
{

foundation::Result<std::pair<std::string, std::string>> parseLine(std::string_view line, std::size_t lineNumber)
{
    const std::string trimmed = foundation::trim(line);

    if (trimmed.empty() || trimmed.front() == '#')
    {
        return foundation::Result<std::pair<std::string, std::string>>(std::make_pair("", ""));
    }

    const auto separator = trimmed.find('=');

    if (separator == std::string::npos)
    {
        return foundation::Result<std::pair<std::string, std::string>>(foundation::Error(
            foundation::ErrorCode::ParseError,
            "Invalid configuration entry at line " + std::to_string(lineNumber) + ": missing '='."));
    }

    std::string key = foundation::trim(trimmed.substr(0, separator));
    std::string value = foundation::trim(trimmed.substr(separator + 1));

    if (key.empty())
    {
        return foundation::Result<std::pair<std::string, std::string>>(foundation::Error(
            foundation::ErrorCode::ParseError,
            "Invalid configuration entry at line " + std::to_string(lineNumber) + ": empty key."));
    }

    return foundation::Result<std::pair<std::string, std::string>>(std::make_pair(std::move(key), std::move(value)));
}

} // namespace

std::string environmentVariableToConfigKey(std::string_view environmentVariable) noexcept
{
    constexpr std::string_view prefix = "SCOPE_";

    if (!foundation::startsWith(environmentVariable, prefix))
    {
        return {};
    }

    std::string configKey = foundation::toLower(environmentVariable.substr(prefix.size()));

    for (char& character : configKey)
    {
        if (character == '_')
        {
            character = '.';
        }
    }

    return configKey;
}

foundation::Result<ConfigurationManager> ConfigurationManager::loadFromFile(const foundation::Path& path)
{
    const auto existsResult = foundation::FileSystem::exists(path);

    if (!existsResult)
    {
        return foundation::Result<ConfigurationManager>(existsResult.error());
    }

    if (!*existsResult)
    {
        return foundation::Result<ConfigurationManager>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Configuration file not found."));
    }

    const auto isFileResult = foundation::FileSystem::isFile(path);

    if (!isFileResult)
    {
        return foundation::Result<ConfigurationManager>(isFileResult.error());
    }

    if (!*isFileResult)
    {
        return foundation::Result<ConfigurationManager>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Configuration path is not a file."));
    }

    std::ifstream stream(path.string());

    if (!stream)
    {
        return foundation::Result<ConfigurationManager>(
            foundation::Error(foundation::ErrorCode::IOError, "Unable to open configuration file."));
    }

    ConfigurationManager manager;
    std::string line;
    std::size_t lineNumber = 0;

    while (std::getline(stream, line))
    {
        ++lineNumber;

        const auto parsed = parseLine(line, lineNumber);

        if (!parsed)
        {
            return foundation::Result<ConfigurationManager>(parsed.error());
        }

        if (!parsed->first.empty())
        {
            manager.m_configuration.set(parsed->first, parsed->second);
        }
    }

    return foundation::Result<ConfigurationManager>(std::move(manager));
}

void ConfigurationManager::applyEnvironment()
{
#if defined(_WIN32)
    for (char** environmentEntry = _environ; environmentEntry != nullptr && *environmentEntry != nullptr;
         ++environmentEntry)
#else
    for (char** environmentEntry = environ; environmentEntry != nullptr && *environmentEntry != nullptr;
         ++environmentEntry)
#endif
    {
        const std::string_view entry(*environmentEntry);
        const auto separator = entry.find('=');

        if (separator == std::string_view::npos)
        {
            continue;
        }

        const std::string_view variableName = entry.substr(0, separator);
        const std::string configKey = environmentVariableToConfigKey(variableName);

        if (configKey.empty())
        {
            continue;
        }

        m_configuration.set(configKey, std::string(entry.substr(separator + 1)));
    }
}

foundation::Result<bool> ConfigurationManager::validate(const std::vector<std::string>& requiredKeys) const
{
    std::vector<std::string> missingKeys;

    for (const std::string& key : requiredKeys)
    {
        if (!m_configuration.has(key))
        {
            missingKeys.push_back(key);
        }
    }

    if (!missingKeys.empty())
    {
        std::ostringstream message;

        message << "Missing required configuration keys:";

        for (const std::string& key : missingKeys)
        {
            message << ' ' << key;
        }

        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, message.str()));
    }

    return foundation::Result<bool>(true);
}

const runtime::Configuration& ConfigurationManager::configuration() const noexcept
{
    return m_configuration;
}

runtime::Configuration& ConfigurationManager::configuration() noexcept
{
    return m_configuration;
}

} // namespace scope::configuration
