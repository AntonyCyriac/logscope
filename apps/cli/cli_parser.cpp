/**
 * @file cli_parser.cpp
 * @brief CLI argument parsing implementation.
 */

#include "cli_parser.hpp"

#include <cstdint>
#include <string_view>

#include "foundation/string.hpp"
#include "foundation/timestamp.hpp"
#include "log_format.hpp"
#include "log_line_classifier.hpp"

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

std::optional<analysis::DetectedLogLevel> parseDetectedLogLevel(const std::string_view value)
{
    const std::string lowered = foundation::toLower(value);

    if (lowered == "error")
    {
        return analysis::DetectedLogLevel::Error;
    }

    if (lowered == "warn" || lowered == "warning")
    {
        return analysis::DetectedLogLevel::Warn;
    }

    if (lowered == "info")
    {
        return analysis::DetectedLogLevel::Info;
    }

    if (lowered == "other")
    {
        return analysis::DetectedLogLevel::Other;
    }

    if (lowered == "blank")
    {
        return analysis::DetectedLogLevel::Blank;
    }

    return std::nullopt;
}

bool parseInvestigationOption(const std::string& argument, int& index, const int argc, char* argv[],
                              investigation::InvestigationCriteria& criteria)
{
    if (argument == "--search" || argument == "--content-search")
    {
        if (index + 1 >= argc)
        {
            return false;
        }

        criteria.contentSearch = argv[++index];

        return true;
    }

    if (argument == "--time-from")
    {
        if (index + 1 >= argc)
        {
            return false;
        }

        const auto timestamp = foundation::Timestamp::parse(argv[++index]);

        if (!timestamp.hasValue())
        {
            return false;
        }

        criteria.timeRange = criteria.timeRange.withEarliest(*timestamp);

        return true;
    }

    if (argument == "--time-to")
    {
        if (index + 1 >= argc)
        {
            return false;
        }

        const auto timestamp = foundation::Timestamp::parse(argv[++index]);

        if (!timestamp.hasValue())
        {
            return false;
        }

        criteria.timeRange = criteria.timeRange.withLatest(*timestamp);

        return true;
    }

    if (argument == "--level")
    {
        if (index + 1 >= argc)
        {
            return false;
        }

        const auto level = parseDetectedLogLevel(argv[++index]);

        if (!level)
        {
            return false;
        }

        criteria.field = criteria.field.withLevel(*level);

        return true;
    }

    if (argument == "--message")
    {
        if (index + 1 >= argc)
        {
            return false;
        }

        criteria.field = criteria.field.withMessageContains(argv[++index]);

        return true;
    }

    if (argument == "--json-key")
    {
        if (index + 1 >= argc)
        {
            return false;
        }

        criteria.field = criteria.field.withRequiredJsonKey(argv[++index]);

        return true;
    }

    return false;
}

bool parseProfileOption(const std::string& argument, int& index, const int argc, char* argv[], std::string& profile)
{
    if (argument == "--profile")
    {
        if (index + 1 >= argc)
        {
            return false;
        }

        profile = argv[++index];

        return true;
    }

    return false;
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

        if (argument == "--log-format")
        {
            if (index + 1 >= argc)
            {
                return std::nullopt;
            }

            const auto logFormat = analysis::parseLogFormat(argv[++index]);

            if (!logFormat || *logFormat == analysis::LogFormat::Unknown)
            {
                return std::nullopt;
            }

            options.logFormat = *logFormat;

            continue;
        }

        if (parseProfileOption(argument, index, argc, argv, options.profile))
        {
            continue;
        }

        if (argument == "--sections")
        {
            if (index + 1 >= argc)
            {
                return std::nullopt;
            }

            const auto sections = reporting::ReportSections::parse(argv[++index]);

            if (!sections)
            {
                return std::nullopt;
            }

            options.sections = *sections;

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

std::optional<InvestigateOptions> parseInvestigateArguments(int argc, char* argv[], int startIndex)
{
    InvestigateOptions options;

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

        if (argument == "--log-format")
        {
            if (index + 1 >= argc)
            {
                return std::nullopt;
            }

            const auto logFormat = analysis::parseLogFormat(argv[++index]);

            if (!logFormat || *logFormat == analysis::LogFormat::Unknown)
            {
                return std::nullopt;
            }

            options.logFormat = *logFormat;

            continue;
        }

        if (parseProfileOption(argument, index, argc, argv, options.profile))
        {
            continue;
        }

        if (parseInvestigationOption(argument, index, argc, argv, options.criteria))
        {
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

std::optional<ExtensionsListOptions> parseExtensionsListArguments(int argc, char* argv[], int startIndex)
{
    ExtensionsListOptions options;

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

        return std::nullopt;
    }

    return options;
}

std::optional<ExtensionsDescribeOptions> parseExtensionsDescribeArguments(int argc, char* argv[], int startIndex)
{
    ExtensionsDescribeOptions options;

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

        if (isOption(argument))
        {
            return std::nullopt;
        }

        if (!options.extensionId.empty())
        {
            return std::nullopt;
        }

        options.extensionId = argument;
    }

    if (options.showHelp)
    {
        return options;
    }

    if (options.extensionId.empty())
    {
        return std::nullopt;
    }

    return options;
}

std::optional<std::uint64_t> parseUnsignedArgument(const char* value)
{
    try
    {
        const std::uint64_t parsed = std::stoull(value);

        return parsed;
    }
    catch (...)
    {
        return std::nullopt;
    }
}

std::optional<SessionSaveOptions> parseSessionSaveArguments(int argc, char* argv[], int startIndex)
{
    SessionSaveOptions options;

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

        if (argument == "--sections")
        {
            if (index + 1 >= argc)
            {
                return std::nullopt;
            }

            const auto sections = reporting::ReportSections::parse(argv[++index]);

            if (!sections)
            {
                return std::nullopt;
            }

            options.sections = *sections;

            continue;
        }

        if (argument == "--min-errors")
        {
            if (index + 1 >= argc)
            {
                return std::nullopt;
            }

            const auto value = parseUnsignedArgument(argv[++index]);

            if (!value)
            {
                return std::nullopt;
            }

            options.minErrors = *value;

            continue;
        }

        if (argument == "--min-warnings")
        {
            if (index + 1 >= argc)
            {
                return std::nullopt;
            }

            const auto value = parseUnsignedArgument(argv[++index]);

            if (!value)
            {
                return std::nullopt;
            }

            options.minWarnings = *value;

            continue;
        }

        if (argument == "--min-lines")
        {
            if (index + 1 >= argc)
            {
                return std::nullopt;
            }

            const auto value = parseUnsignedArgument(argv[++index]);

            if (!value)
            {
                return std::nullopt;
            }

            options.minLines = *value;

            continue;
        }

        if (argument == "--max-lines")
        {
            if (index + 1 >= argc)
            {
                return std::nullopt;
            }

            const auto value = parseUnsignedArgument(argv[++index]);

            if (!value)
            {
                return std::nullopt;
            }

            options.maxLines = *value;

            continue;
        }

        if (argument == "--search")
        {
            if (index + 1 >= argc)
            {
                return std::nullopt;
            }

            options.searchQuery = argv[++index];

            continue;
        }

        if (parseProfileOption(argument, index, argc, argv, options.profile))
        {
            continue;
        }

        if (parseInvestigationOption(argument, index, argc, argv, options.contentCriteria))
        {
            continue;
        }

        if (isOption(argument))
        {
            return std::nullopt;
        }

        if (options.sessionFile.string().empty())
        {
            options.sessionFile = foundation::Path(argument);

            continue;
        }

        if (options.logFile.string().empty())
        {
            options.logFile = foundation::Path(argument);

            continue;
        }

        return std::nullopt;
    }

    if (options.showHelp)
    {
        return options;
    }

    if (options.sessionFile.string().empty() || options.logFile.string().empty())
    {
        return std::nullopt;
    }

    return options;
}

std::optional<SessionLoadOptions> parseSessionLoadArguments(int argc, char* argv[], int startIndex)
{
    SessionLoadOptions options;

    for (int index = startIndex; index < argc; ++index)
    {
        const std::string argument = argv[index];

        if (argument == "--help" || argument == "-h")
        {
            options.showHelp = true;

            return options;
        }

        if (isOption(argument))
        {
            return std::nullopt;
        }

        if (!options.sessionFile.string().empty())
        {
            return std::nullopt;
        }

        options.sessionFile = foundation::Path(argument);
    }

    if (options.showHelp)
    {
        return options;
    }

    if (options.sessionFile.string().empty())
    {
        return std::nullopt;
    }

    return options;
}

std::optional<SessionListOptions> parseSessionListArguments(int argc, char* argv[], int startIndex)
{
    SessionListOptions options;

    for (int index = startIndex; index < argc; ++index)
    {
        const std::string argument = argv[index];

        if (argument == "--help" || argument == "-h")
        {
            options.showHelp = true;

            return options;
        }

        if (isOption(argument))
        {
            return std::nullopt;
        }

        if (!options.directory.string().empty() && options.directory.string() != ".")
        {
            return std::nullopt;
        }

        options.directory = foundation::Path(argument);
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

        if (argc >= 3 && std::string_view(argv[2]) == "investigate")
        {
            parsed.command = CliCommand::Investigate;
            parsed.investigate.showHelp = true;

            return parsed;
        }

        if (argc >= 4 && std::string_view(argv[2]) == "config" && std::string_view(argv[3]) == "validate")
        {
            parsed.command = CliCommand::ConfigValidate;
            parsed.configValidate.showHelp = true;

            return parsed;
        }

        if (argc >= 4 && std::string_view(argv[2]) == "extensions" && std::string_view(argv[3]) == "list")
        {
            parsed.command = CliCommand::ExtensionsList;
            parsed.extensionsList.showHelp = true;

            return parsed;
        }

        if (argc >= 4 && std::string_view(argv[2]) == "extensions" && std::string_view(argv[3]) == "describe")
        {
            parsed.command = CliCommand::ExtensionsDescribe;
            parsed.extensionsDescribe.showHelp = true;

            return parsed;
        }

        if (argc >= 4 && std::string_view(argv[2]) == "session" && std::string_view(argv[3]) == "save")
        {
            parsed.command = CliCommand::SessionSave;
            parsed.sessionSave.showHelp = true;

            return parsed;
        }

        if (argc >= 4 && std::string_view(argv[2]) == "session" && std::string_view(argv[3]) == "load")
        {
            parsed.command = CliCommand::SessionLoad;
            parsed.sessionLoad.showHelp = true;

            return parsed;
        }

        if (argc >= 4 && std::string_view(argv[2]) == "session" && std::string_view(argv[3]) == "list")
        {
            parsed.command = CliCommand::SessionList;
            parsed.sessionList.showHelp = true;

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

    if (firstArgument == "investigate")
    {
        const auto options = parseInvestigateArguments(argc, argv, 2);

        if (!options)
        {
            return std::nullopt;
        }

        parsed.command = CliCommand::Investigate;
        parsed.investigate = *options;

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

    if (firstArgument == "extensions")
    {
        if (argc < 3)
        {
            return std::nullopt;
        }

        const std::string_view subcommand = argv[2];

        if (subcommand == "list")
        {
            const auto options = parseExtensionsListArguments(argc, argv, 3);

            if (!options)
            {
                return std::nullopt;
            }

            parsed.command = CliCommand::ExtensionsList;
            parsed.extensionsList = *options;

            return parsed;
        }

        if (subcommand == "describe")
        {
            const auto options = parseExtensionsDescribeArguments(argc, argv, 3);

            if (!options)
            {
                return std::nullopt;
            }

            parsed.command = CliCommand::ExtensionsDescribe;
            parsed.extensionsDescribe = *options;

            return parsed;
        }

        return std::nullopt;
    }

    if (firstArgument == "session")
    {
        if (argc < 3)
        {
            return std::nullopt;
        }

        const std::string_view subcommand = argv[2];

        if (subcommand == "save")
        {
            const auto options = parseSessionSaveArguments(argc, argv, 3);

            if (!options)
            {
                return std::nullopt;
            }

            parsed.command = CliCommand::SessionSave;
            parsed.sessionSave = *options;

            return parsed;
        }

        if (subcommand == "load")
        {
            const auto options = parseSessionLoadArguments(argc, argv, 3);

            if (!options)
            {
                return std::nullopt;
            }

            parsed.command = CliCommand::SessionLoad;
            parsed.sessionLoad = *options;

            return parsed;
        }

        if (subcommand == "list")
        {
            const auto options = parseSessionListArguments(argc, argv, 3);

            if (!options)
            {
                return std::nullopt;
            }

            parsed.command = CliCommand::SessionList;
            parsed.sessionList = *options;

            return parsed;
        }

        return std::nullopt;
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
