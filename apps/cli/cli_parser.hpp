/**
 * @file cli_parser.hpp
 * @brief Parses CLI arguments into command options.
 */

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "foundation/path.hpp"
#include "output_format.hpp"
#include "report_section.hpp"

namespace scope::cli
{

/**
 * @brief Supported top-level CLI commands.
 */
enum class CliCommand
{
    Help,
    Analyze,
    ConfigValidate,
    ExtensionsList,
    ExtensionsDescribe
};

/**
 * @brief Options for the analyze command.
 */
struct AnalyzeOptions
{
    foundation::Path logFile;
    foundation::Path configFile;
    OutputFormat format = OutputFormat::Text;
    std::optional<reporting::ReportSections> sections;
    bool showHelp = false;
};

/**
 * @brief Options for the config validate command.
 */
struct ConfigValidateOptions
{
    foundation::Path configFile;
    std::vector<std::string> requiredKeys;
    bool showHelp = false;
};

/**
 * @brief Options for the extensions list command.
 */
struct ExtensionsListOptions
{
    foundation::Path configFile;
    bool showHelp = false;
};

/**
 * @brief Options for the extensions describe command.
 */
struct ExtensionsDescribeOptions
{
    foundation::Path configFile;
    std::string extensionId;
    bool showHelp = false;
};

/**
 * @brief Parsed CLI invocation.
 */
struct ParsedCli
{
    CliCommand command = CliCommand::Help;
    AnalyzeOptions analyze;
    ConfigValidateOptions configValidate;
    ExtensionsListOptions extensionsList;
    ExtensionsDescribeOptions extensionsDescribe;
    bool showGlobalHelp = false;
};

/**
 * @brief Parses command-line arguments.
 */
[[nodiscard]] std::optional<ParsedCli> parseCliArguments(int argc, char* argv[]);

} // namespace scope::cli
