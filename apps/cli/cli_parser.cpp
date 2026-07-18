/**
 * @file cli_parser.cpp
 * @brief CLI argument parsing implementation.
 */

#include "cli_parser.hpp"

#include <string_view>

#include "foundation/string.hpp"

namespace scope::cli
{

namespace
{

bool isOption(const std::string& argument) noexcept
{
    return argument.size() > 1 && argument.front() == '-';
}

std::vector<std::string> splitRequiredKeys(std::string_view value)
{
    std::vector<std::string> keys;
    const std::vector<std::string> parts = foundation::split(value, ',');

    for (const std::string& part : parts)
    {
        const std::string trimmed = foundation::trim(part);

        if (!trimmed.empty())
        {
            keys.push_back(trimmed);
        }
    }

    return keys;
}

std::optional<AnalyzeOptions> parseAnalyzeArguments(int argc, char* argv[], int startIndex)
{
    AnalyzeOptions options;

    for (int index = startIndex; index < argc; ++index)
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

            options.configFile = foundation::Path(argv[++index]);

            continue;
        }

        if (argument == "--format")
        {
            if (index + 1 >= argc)
            {
                return std::nullopt;
            }

            const auto format = parseOutputFormat(argv[++index]);

            if (!format)
            {
                return std::nullopt;
            }

            options.format = *format;

            continue;
        }

        if (isOption(argument))
        {
            return std::nullopt;
        }

        if (!options.logFile.string().empty())
        {
            return std::nullopt;
        }

        options.logFile = foundation::Path(argument);
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

std::optional<ConfigValidateOptions> parseConfigValidateArguments(int argc, char* argv[], int startIndex)
{
    ConfigValidateOptions options;

    for (int index = startIndex; index < argc; ++index)
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

            options.configFile = foundation::Path(argv[++index]);

            continue;
        }

        if (argument == "--require")
        {
            if (index + 1 >= argc)
            {
                return std::nullopt;
            }

            options.requiredKeys = splitRequiredKeys(argv[++index]);

            continue;
        }

        return std::nullopt;
    }

    return options;
}

} // namespace

std::optional<ParsedCli> parseCliArguments(int argc, char* argv[])
{
    if (argc <= 1)
    {
        return std::nullopt;
    }

    ParsedCli parsed;
    const std::string firstArgument = argv[1];

    if (firstArgument == "--help" || firstArgument == "-h")
    {
        parsed.showGlobalHelp = true;

        return parsed;
    }

    if (firstArgument == "help")
    {
        if (argc >= 3 && std::string_view(argv[2]) == "analyze")
        {
            parsed.command = CliCommand::Analyze;
            parsed.analyze.showHelp = true;

            return parsed;
        }

        if (argc >= 4 && std::string_view(argv[2]) == "config" && std::string_view(argv[3]) == "validate")
        {
            parsed.command = CliCommand::ConfigValidate;
            parsed.configValidate.showHelp = true;

            return parsed;
        }

        parsed.showGlobalHelp = true;

        return parsed;
    }

    if (firstArgument == "analyze")
    {
        const auto options = parseAnalyzeArguments(argc, argv, 2);

        if (!options)
        {
            return std::nullopt;
        }

        parsed.command = CliCommand::Analyze;
        parsed.analyze = *options;

        return parsed;
    }

    if (firstArgument == "config")
    {
        if (argc < 3 || std::string_view(argv[2]) != "validate")
        {
            return std::nullopt;
        }

        const auto options = parseConfigValidateArguments(argc, argv, 3);

        if (!options)
        {
            return std::nullopt;
        }

        parsed.command = CliCommand::ConfigValidate;
        parsed.configValidate = *options;

        return parsed;
    }

    const auto legacyOptions = parseAnalyzeArguments(argc, argv, 1);

    if (!legacyOptions)
    {
        return std::nullopt;
    }

    parsed.command = CliCommand::Analyze;
    parsed.analyze = *legacyOptions;

    return parsed;
}

} // namespace scope::cli
