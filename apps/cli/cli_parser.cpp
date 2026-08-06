/**
 * @file cli_parser.cpp
 * @brief CLI argument parsing implementation.
 */

#include "cli_parser.hpp"

#include <cstdint>
#include <string_view>

#include "analytics_command.hpp"
#include "foundation/string.hpp"
#include "foundation/timestamp.hpp"
#include "log_format.hpp"
#include "log_line_classifier.hpp"
#include "search_query.hpp"

namespace scope::cli
{

namespace
{

std::optional<std::uint64_t> parseUnsignedArgument(const char* value);

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

bool parseStorageOption(const std::string& argument, int& index, const int argc, char* argv[], bool& persistIndex,
                        bool& reuseIndex, std::optional<foundation::Path>& indexPath)
{
    if (argument == "--persist-index")
    {
        persistIndex = true;

        return true;
    }

    if (argument == "--reuse-index")
    {
        reuseIndex = true;

        return true;
    }

    if (argument == "--index-path")
    {
        if (index + 1 >= argc)
        {
            return false;
        }

        indexPath = foundation::Path(argv[++index]);

        return true;
    }

    return false;
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

    if (argument == "--query")
    {
        if (index + 1 >= argc)
        {
            return false;
        }

        criteria.booleanQuery = argv[++index];

        return true;
    }

    if (argument == "--filter")
    {
        if (index + 1 >= argc)
        {
            return false;
        }

        criteria.filterExpression = argv[++index];

        return true;
    }

    if (argument == "--regex")
    {
        criteria.searchMode = search::SearchMode::Regex;

        return true;
    }

    if (argument == "--case-sensitive")
    {
        criteria.caseSensitivity = search::CaseSensitivity::Sensitive;

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

        if (parseStorageOption(argument, index, argc, argv, options.persistIndex, options.reuseIndex, options.indexPath))
        {
            continue;
        }

        if (argument == "--stats")
        {
            options.showStats = true;

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

        if (argument == "--output")
        {
            if (index + 1 >= argc)
            {
                return std::nullopt;
            }

            options.outputFile = foundation::Path(argv[++index]);

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

        if (parseStorageOption(argument, index, argc, argv, options.persistIndex, options.reuseIndex, options.indexPath))
        {
            continue;
        }

        if (argument == "--stats")
        {
            options.showStats = true;

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

std::optional<AgentInvestigateOptions> parseAgentInvestigateArguments(int argc, char* argv[], int startIndex)
{
    AgentInvestigateOptions options;

    for (int index = startIndex; index < argc; ++index)
    {
        const std::string argument = argv[index];

        if (argument == "--help" || argument == "-h")
        {
            options.investigate.showHelp = true;

            return options;
        }

        if (argument == "--ask")
        {
            if (index + 1 >= argc)
            {
                return std::nullopt;
            }

            options.askQuery = argv[++index];

            continue;
        }

        if (argument == "--summarize")
        {
            options.summarize = true;

            continue;
        }

        if (argument == "--hints")
        {
            options.hints = true;

            continue;
        }

        if (argument == "--stats")
        {
            options.investigate.showStats = true;

            continue;
        }

        if (argument == "--config")
        {
            if (index + 1 >= argc)
            {
                return std::nullopt;
            }

            options.investigate.configFile = foundation::Path(argv[++index]);

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

            options.investigate.format = *format;

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

            options.investigate.logFormat = *logFormat;

            continue;
        }

        if (parseProfileOption(argument, index, argc, argv, options.investigate.profile))
        {
            continue;
        }

        if (parseStorageOption(argument, index, argc, argv, options.investigate.persistIndex,
                               options.investigate.reuseIndex, options.investigate.indexPath))
        {
            continue;
        }

        if (parseInvestigationOption(argument, index, argc, argv, options.investigate.criteria))
        {
            continue;
        }

        if (isOption(argument))
        {
            return std::nullopt;
        }

        if (!options.investigate.logFile.string().empty())
        {
            return std::nullopt;
        }

        options.investigate.logFile = foundation::Path(argument);
    }

    if (options.investigate.showHelp)
    {
        return options;
    }

    if (options.investigate.logFile.string().empty())
    {
        return std::nullopt;
    }

    return options;
}

std::optional<AnalyticsOptions> parseAnalyticsArguments(int argc, char* argv[], int startIndex)
{
    AnalyticsOptions options;

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

        if (argument == "--bucket")
        {
            if (index + 1 >= argc)
            {
                return std::nullopt;
            }

            try
            {
                options.analyticsConfig.bucketSeconds = std::stoll(argv[++index]);
            }
            catch (...)
            {
                return std::nullopt;
            }

            continue;
        }

        if (argument == "--top")
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

            options.analyticsConfig.topN = static_cast<std::size_t>(*value);

            continue;
        }

        if (parseProfileOption(argument, index, argc, argv, options.profile))
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

        if (parseStorageOption(argument, index, argc, argv, options.persistIndex, options.reuseIndex, options.indexPath))
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

        if (argument == "--output")
        {
            if (index + 1 >= argc)
            {
                return std::nullopt;
            }

            options.outputFile = foundation::Path(argv[++index]);

            continue;
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

std::optional<InvestigationCreateOptions> parseInvestigationCreateArguments(int argc, char* argv[], int startIndex)
{
    InvestigationCreateOptions options;

    for (int index = startIndex; index < argc; ++index)
    {
        const std::string argument = argv[index];

        if (argument == "--help" || argument == "-h")
        {
            options.showHelp = true;

            return options;
        }

        if (argument == "--name" && index + 1 < argc)
        {
            options.name = argv[++index];
            continue;
        }

        if (argument == "--description" && index + 1 < argc)
        {
            options.description = argv[++index];
            continue;
        }

        if (argument == "--dir" && index + 1 < argc)
        {
            options.rootDirectory = foundation::Path(argv[++index]);
            continue;
        }

        return std::nullopt;
    }

    return options;
}

std::optional<InvestigationAddOptions> parseInvestigationAddArguments(int argc, char* argv[], int startIndex)
{
    InvestigationAddOptions options;

    for (int index = startIndex; index < argc; ++index)
    {
        const std::string argument = argv[index];

        if (argument == "--help" || argument == "-h")
        {
            options.showHelp = true;

            return options;
        }

        if (argument == "--display-name" && index + 1 < argc)
        {
            options.displayName = argv[++index];
            continue;
        }

        if (argument == "--type" && index + 1 < argc)
        {
            options.artifactType = argv[++index];
            continue;
        }

        if (argument == "--role" && index + 1 < argc)
        {
            options.role = argv[++index];
            continue;
        }

        if (argument == "--dir" && index + 1 < argc)
        {
            options.rootDirectory = foundation::Path(argv[++index]);
            continue;
        }

        if (isOption(argument))
        {
            return std::nullopt;
        }

        if (options.investigationId.empty())
        {
            options.investigationId = argument;
            continue;
        }

        if (options.logFile.string().empty())
        {
            options.logFile = foundation::Path(argument);
            continue;
        }

        return std::nullopt;
    }

    if (options.investigationId.empty() || options.logFile.string().empty())
    {
        return std::nullopt;
    }

    return options;
}

std::optional<InvestigationAddNoteOptions> parseInvestigationAddNoteArguments(int argc, char* argv[], int startIndex)
{
    InvestigationAddNoteOptions options;

    for (int index = startIndex; index < argc; ++index)
    {
        const std::string argument = argv[index];

        if (argument == "--help" || argument == "-h")
        {
            options.showHelp = true;

            return options;
        }

        if (argument == "--title" && index + 1 < argc)
        {
            options.title = argv[++index];
            continue;
        }

        if (argument == "--body" && index + 1 < argc)
        {
            options.body = argv[++index];
            continue;
        }

        if (argument == "--dir" && index + 1 < argc)
        {
            options.rootDirectory = foundation::Path(argv[++index]);
            continue;
        }

        if (isOption(argument))
        {
            return std::nullopt;
        }

        if (options.investigationId.empty())
        {
            options.investigationId = argument;
            continue;
        }

        return std::nullopt;
    }

    if (options.investigationId.empty())
    {
        return std::nullopt;
    }

    return options;
}

std::optional<InvestigationListOptions> parseInvestigationListArguments(int argc, char* argv[], int startIndex)
{
    InvestigationListOptions options;

    for (int index = startIndex; index < argc; ++index)
    {
        const std::string argument = argv[index];

        if (argument == "--help" || argument == "-h")
        {
            options.showHelp = true;

            return options;
        }

        if (argument == "--dir" && index + 1 < argc)
        {
            options.rootDirectory = foundation::Path(argv[++index]);
            continue;
        }

        return std::nullopt;
    }

    return options;
}

std::optional<InvestigationShowOptions> parseInvestigationShowArguments(int argc, char* argv[], int startIndex)
{
    InvestigationShowOptions options;

    for (int index = startIndex; index < argc; ++index)
    {
        const std::string argument = argv[index];

        if (argument == "--help" || argument == "-h")
        {
            options.showHelp = true;

            return options;
        }

        if (argument == "--dir" && index + 1 < argc)
        {
            options.rootDirectory = foundation::Path(argv[++index]);
            continue;
        }

        if (isOption(argument))
        {
            return std::nullopt;
        }

        if (options.investigationId.empty())
        {
            options.investigationId = argument;
            continue;
        }

        return std::nullopt;
    }

    if (options.investigationId.empty())
    {
        return std::nullopt;
    }

    return options;
}

std::optional<InvestigationOpenOptions> parseInvestigationOpenArguments(int argc, char* argv[], int startIndex)
{
    InvestigationOpenOptions options;

    for (int index = startIndex; index < argc; ++index)
    {
        const std::string argument = argv[index];

        if (argument == "--help" || argument == "-h")
        {
            options.showHelp = true;

            return options;
        }

        if (argument == "--artifact" && index + 1 < argc)
        {
            options.artifactId = argv[++index];
            continue;
        }

        if (argument == "--dir" && index + 1 < argc)
        {
            options.rootDirectory = foundation::Path(argv[++index]);
            continue;
        }

        if (isOption(argument))
        {
            return std::nullopt;
        }

        if (options.investigationId.empty())
        {
            options.investigationId = argument;
            continue;
        }

        return std::nullopt;
    }

    if (options.investigationId.empty())
    {
        return std::nullopt;
    }

    return options;
}

std::optional<InvestigationTimelineFormat> parseInvestigationTimelineFormat(const std::string& value)
{
    if (value == "json" || value == "JSON")
    {
        return InvestigationTimelineFormat::Json;
    }

    if (value == "table" || value == "TABLE")
    {
        return InvestigationTimelineFormat::Table;
    }

    return std::nullopt;
}

std::optional<scope::workspace::TimelineSortOrder> parseTimelineSortOrder(const std::string& value)
{
    if (value == "asc" || value == "ASC")
    {
        return scope::workspace::TimelineSortOrder::Ascending;
    }

    if (value == "desc" || value == "DESC")
    {
        return scope::workspace::TimelineSortOrder::Descending;
    }

    return std::nullopt;
}

std::optional<InvestigationTimelineOptions> parseInvestigationTimelineArguments(int argc, char* argv[], int startIndex)
{
    InvestigationTimelineOptions options;

    for (int index = startIndex; index < argc; ++index)
    {
        const std::string argument = argv[index];

        if (argument == "--help" || argument == "-h")
        {
            options.showHelp = true;

            return options;
        }

        if (argument == "--format" && index + 1 < argc)
        {
            const auto format = parseInvestigationTimelineFormat(argv[++index]);

            if (!format)
            {
                return std::nullopt;
            }

            options.format = *format;
            continue;
        }

        if (argument == "--limit" && index + 1 < argc)
        {
            try
            {
                options.limit = static_cast<std::size_t>(std::stoull(argv[++index]));
            }
            catch (...)
            {
                return std::nullopt;
            }

            continue;
        }

        if (argument == "--order" && index + 1 < argc)
        {
            const auto order = parseTimelineSortOrder(argv[++index]);

            if (!order)
            {
                return std::nullopt;
            }

            options.order = *order;
            continue;
        }

        if (argument == "--dir" && index + 1 < argc)
        {
            options.rootDirectory = foundation::Path(argv[++index]);
            continue;
        }

        if (isOption(argument))
        {
            return std::nullopt;
        }

        if (options.investigationId.empty())
        {
            options.investigationId = argument;
            continue;
        }

        return std::nullopt;
    }

    if (options.investigationId.empty())
    {
        return std::nullopt;
    }

    return options;
}

std::optional<InvestigationCrashOptions> parseInvestigationCrashArguments(int argc, char* argv[], int startIndex)
{
    InvestigationCrashOptions options;

    for (int index = startIndex; index < argc; ++index)
    {
        const std::string argument = argv[index];

        if (argument == "--help" || argument == "-h")
        {
            options.showHelp = true;

            return options;
        }

        if (argument == "--format" && index + 1 < argc)
        {
            const auto format = parseInvestigationTimelineFormat(argv[++index]);

            if (!format)
            {
                return std::nullopt;
            }

            options.format = *format;
            continue;
        }

        if (argument == "--artifact" && index + 1 < argc)
        {
            options.artifactId = argv[++index];
            continue;
        }

        if (argument == "--dir" && index + 1 < argc)
        {
            options.rootDirectory = foundation::Path(argv[++index]);
            continue;
        }

        if (isOption(argument))
        {
            return std::nullopt;
        }

        if (options.investigationId.empty())
        {
            options.investigationId = argument;
            continue;
        }

        return std::nullopt;
    }

    if (options.investigationId.empty())
    {
        return std::nullopt;
    }

    return options;
}

std::optional<InvestigationLinksOptions> parseInvestigationLinksArguments(int argc, char* argv[], const int linksVerbIndex)
{
    if (linksVerbIndex >= argc)
    {
        return std::nullopt;
    }

    InvestigationLinksOptions options;
    const std::string linksVerb = argv[linksVerbIndex];

    if (linksVerb == "list")
    {
        options.subcommand = InvestigationLinksSubcommand::List;
    }
    else if (linksVerb == "add")
    {
        options.subcommand = InvestigationLinksSubcommand::Add;
    }
    else if (linksVerb == "remove")
    {
        options.subcommand = InvestigationLinksSubcommand::Remove;
    }
    else
    {
        return std::nullopt;
    }

    for (int index = linksVerbIndex + 1; index < argc; ++index)
    {
        const std::string argument = argv[index];

        if (argument == "--help" || argument == "-h")
        {
            options.showHelp = true;

            return options;
        }

        if (argument == "--format" && index + 1 < argc)
        {
            const auto format = parseInvestigationTimelineFormat(argv[++index]);

            if (!format)
            {
                return std::nullopt;
            }

            options.format = *format;
            continue;
        }

        if (argument == "--dir" && index + 1 < argc)
        {
            options.rootDirectory = foundation::Path(argv[++index]);
            continue;
        }

        if (argument == "--source" && index + 1 < argc)
        {
            options.sourceEventId = argv[++index];
            continue;
        }

        if (argument == "--target" && index + 1 < argc)
        {
            options.targetEventId = argv[++index];
            continue;
        }

        if (argument == "--type" && index + 1 < argc)
        {
            options.linkType = argv[++index];
            continue;
        }

        if (argument == "--note" && index + 1 < argc)
        {
            options.note = argv[++index];
            continue;
        }

        if (argument == "--link" && index + 1 < argc)
        {
            options.linkId = argv[++index];
            continue;
        }

        if (isOption(argument))
        {
            return std::nullopt;
        }

        if (options.investigationId.empty())
        {
            options.investigationId = argument;
            continue;
        }

        return std::nullopt;
    }

    if (options.investigationId.empty())
    {
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

        if (argc >= 4 && std::string_view(argv[2]) == "investigation" && std::string_view(argv[3]) == "create")
        {
            parsed.command = CliCommand::InvestigationCreate;
            parsed.investigationCreate.showHelp = true;

            return parsed;
        }

        if (argc >= 4 && std::string_view(argv[2]) == "investigation" && std::string_view(argv[3]) == "add")
        {
            parsed.command = CliCommand::InvestigationAdd;
            parsed.investigationAdd.showHelp = true;

            return parsed;
        }

        if (argc >= 4 && std::string_view(argv[2]) == "investigation" && std::string_view(argv[3]) == "add-note")
        {
            parsed.command = CliCommand::InvestigationAddNote;
            parsed.investigationAddNote.showHelp = true;

            return parsed;
        }

        if (argc >= 4 && std::string_view(argv[2]) == "investigation" && std::string_view(argv[3]) == "list")
        {
            parsed.command = CliCommand::InvestigationList;
            parsed.investigationList.showHelp = true;

            return parsed;
        }

        if (argc >= 4 && std::string_view(argv[2]) == "investigation" && std::string_view(argv[3]) == "show")
        {
            parsed.command = CliCommand::InvestigationShow;
            parsed.investigationShow.showHelp = true;

            return parsed;
        }

        if (argc >= 4 && std::string_view(argv[2]) == "investigation" && std::string_view(argv[3]) == "open")
        {
            parsed.command = CliCommand::InvestigationOpen;
            parsed.investigationOpen.showHelp = true;

            return parsed;
        }

        if (argc >= 4 && std::string_view(argv[2]) == "investigation" && std::string_view(argv[3]) == "timeline")
        {
            parsed.command = CliCommand::InvestigationTimeline;
            parsed.investigationTimeline.showHelp = true;

            return parsed;
        }

        if (argc >= 4 && std::string_view(argv[2]) == "investigation" && std::string_view(argv[3]) == "crash")
        {
            parsed.command = CliCommand::InvestigationCrash;
            parsed.investigationCrash.showHelp = true;

            return parsed;
        }

        if (argc >= 5 && std::string_view(argv[2]) == "investigation" && std::string_view(argv[3]) == "links")
        {
            parsed.command = CliCommand::InvestigationLinks;
            parsed.investigationLinks.showHelp = true;

            return parsed;
        }

        if (argc >= 3 && std::string_view(argv[2]) == "agent")
        {
            parsed.command = CliCommand::AgentHelp;

            return parsed;
        }

        if (argc >= 4 && std::string_view(argv[2]) == "agent" && std::string_view(argv[3]) == "investigate")
        {
            parsed.command = CliCommand::AgentInvestigate;
            parsed.agentInvestigate.investigate.showHelp = true;

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

    if (firstArgument == "search")
    {
        const auto options = parseInvestigateArguments(argc, argv, 2);

        if (!options)
        {
            return std::nullopt;
        }

        parsed.command = CliCommand::Search;
        parsed.search = *options;

        return parsed;
    }

    if (firstArgument == "query")
    {
        const auto options = parseInvestigateArguments(argc, argv, 2);

        if (!options)
        {
            return std::nullopt;
        }

        parsed.command = CliCommand::Query;
        parsed.query = *options;

        return parsed;
    }

    if (firstArgument == "analytics")
    {
        const auto options = parseAnalyticsArguments(argc, argv, 2);

        if (!options)
        {
            return std::nullopt;
        }

        parsed.command = CliCommand::Analytics;
        parsed.analytics = *options;

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

    if (firstArgument == "investigation")
    {
        if (argc < 3)
        {
            return std::nullopt;
        }

        const std::string_view subcommand = argv[2];

        if (subcommand == "create")
        {
            const auto options = parseInvestigationCreateArguments(argc, argv, 3);

            if (!options)
            {
                return std::nullopt;
            }

            parsed.command = CliCommand::InvestigationCreate;
            parsed.investigationCreate = *options;

            return parsed;
        }

        if (subcommand == "add")
        {
            const auto options = parseInvestigationAddArguments(argc, argv, 3);

            if (!options)
            {
                return std::nullopt;
            }

            parsed.command = CliCommand::InvestigationAdd;
            parsed.investigationAdd = *options;

            return parsed;
        }

        if (subcommand == "add-note")
        {
            const auto options = parseInvestigationAddNoteArguments(argc, argv, 3);

            if (!options)
            {
                return std::nullopt;
            }

            parsed.command = CliCommand::InvestigationAddNote;
            parsed.investigationAddNote = *options;

            return parsed;
        }

        if (subcommand == "list")
        {
            const auto options = parseInvestigationListArguments(argc, argv, 3);

            if (!options)
            {
                return std::nullopt;
            }

            parsed.command = CliCommand::InvestigationList;
            parsed.investigationList = *options;

            return parsed;
        }

        if (subcommand == "show")
        {
            const auto options = parseInvestigationShowArguments(argc, argv, 3);

            if (!options)
            {
                return std::nullopt;
            }

            parsed.command = CliCommand::InvestigationShow;
            parsed.investigationShow = *options;

            return parsed;
        }

        if (subcommand == "open")
        {
            const auto options = parseInvestigationOpenArguments(argc, argv, 3);

            if (!options)
            {
                return std::nullopt;
            }

            parsed.command = CliCommand::InvestigationOpen;
            parsed.investigationOpen = *options;

            return parsed;
        }

        if (subcommand == "timeline")
        {
            const auto options = parseInvestigationTimelineArguments(argc, argv, 3);

            if (!options)
            {
                return std::nullopt;
            }

            parsed.command = CliCommand::InvestigationTimeline;
            parsed.investigationTimeline = *options;

            return parsed;
        }

        if (subcommand == "crash")
        {
            const auto options = parseInvestigationCrashArguments(argc, argv, 3);

            if (!options)
            {
                return std::nullopt;
            }

            parsed.command = CliCommand::InvestigationCrash;
            parsed.investigationCrash = *options;

            return parsed;
        }

        if (subcommand == "links")
        {
            const auto options = parseInvestigationLinksArguments(argc, argv, 4);

            if (!options)
            {
                return std::nullopt;
            }

            parsed.command = CliCommand::InvestigationLinks;
            parsed.investigationLinks = *options;

            return parsed;
        }

        return std::nullopt;
    }

    if (firstArgument == "agent")
    {
        if (argc < 3)
        {
            return std::nullopt;
        }

        const std::string_view subcommand = argv[2];

        if (subcommand == "--help" || subcommand == "-h")
        {
            parsed.command = CliCommand::AgentHelp;

            return parsed;
        }

        if (subcommand == "investigate")
        {
            const auto options = parseAgentInvestigateArguments(argc, argv, 3);

            if (!options)
            {
                return std::nullopt;
            }

            parsed.command = CliCommand::AgentInvestigate;
            parsed.agentInvestigate = *options;

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
