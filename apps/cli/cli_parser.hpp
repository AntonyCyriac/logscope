/**
 * @file cli_parser.hpp
 * @brief Parses CLI arguments into command options.
 */

#pragma once

#include <optional>
#include <string>
#include <vector>

#include <cstdint>

#include "foundation/path.hpp"
#include "investigation_criteria.hpp"
#include "timeline_event.hpp"
#include "log_format.hpp"
#include "output_format.hpp"
#include "report_section.hpp"

#include "analytics_command.hpp"

namespace scope::cli
{

/**
 * @brief Supported top-level CLI commands.
 */
enum class CliCommand
{
    Help,
    Analyze,
    Investigate,
    Search,
    Query,
    Analytics,
    ConfigValidate,
    ExtensionsList,
    ExtensionsDescribe,
    SessionSave,
    SessionLoad,
    SessionList,
    InvestigationCreate,
    InvestigationAdd,
    InvestigationAddNote,
    InvestigationList,
    InvestigationShow,
    InvestigationOpen,
    InvestigationTimeline,
    AgentHelp,
    AgentInvestigate
};

/**
 * @brief Options for the analyze command.
 */
struct AnalyzeOptions
{
    foundation::Path logFile;
    foundation::Path configFile;
    std::optional<foundation::Path> outputFile;
    std::optional<OutputFormat> format;
    analysis::LogFormat logFormat = analysis::LogFormat::Auto;
    std::string profile;
    std::optional<reporting::ReportSections> sections;
    bool persistIndex = false;
    bool reuseIndex = false;
    std::optional<foundation::Path> indexPath;
    bool showStats = false;
    bool showHelp = false;
};

/**
 * @brief Options for the investigate command.
 */
struct InvestigateOptions
{
    foundation::Path logFile;
    foundation::Path configFile;
    OutputFormat format = OutputFormat::Text;
    analysis::LogFormat logFormat = analysis::LogFormat::Auto;
    std::string profile;
    investigation::InvestigationCriteria criteria;
    bool persistIndex = false;
    bool reuseIndex = false;
    std::optional<foundation::Path> indexPath;
    bool showStats = false;
    bool showHelp = false;
};

/**
 * @brief Options for the agent investigate command.
 */
struct AgentInvestigateOptions
{
    InvestigateOptions investigate;
    std::string askQuery;
    bool summarize = false;
    bool hints = false;
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
 * @brief Options for the session save command.
 */
struct SessionSaveOptions
{
    foundation::Path sessionFile;
    foundation::Path logFile;
    foundation::Path configFile;
    OutputFormat format = OutputFormat::Text;
    std::optional<reporting::ReportSections> sections;
    std::uint64_t minErrors = 0U;
    std::uint64_t minWarnings = 0U;
    std::uint64_t minLines = 0U;
    std::optional<std::uint64_t> maxLines;
    std::string searchQuery;
    std::string profile;
    investigation::InvestigationCriteria contentCriteria;
    bool persistIndex = false;
    bool reuseIndex = false;
    std::optional<foundation::Path> indexPath;
    bool showHelp = false;
};

/**
 * @brief Options for the session load command.
 */
struct SessionLoadOptions
{
    foundation::Path sessionFile;
    std::optional<foundation::Path> outputFile;
    bool showHelp = false;
};

/**
 * @brief Options for the session list command.
 */
struct SessionListOptions
{
    foundation::Path directory = foundation::Path(".");
    bool showHelp = false;
};

struct InvestigationCreateOptions
{
    std::string name;
    std::string description;
    foundation::Path rootDirectory = foundation::Path("workspaces");
    bool showHelp = false;
};

struct InvestigationAddOptions
{
    std::string investigationId;
    foundation::Path logFile;
    std::string displayName;
    std::string artifactType;
    std::string role;
    foundation::Path rootDirectory = foundation::Path("workspaces");
    bool showHelp = false;
};

struct InvestigationAddNoteOptions
{
    std::string investigationId;
    std::string title;
    std::string body;
    foundation::Path rootDirectory = foundation::Path("workspaces");
    bool showHelp = false;
};

struct InvestigationListOptions
{
    foundation::Path rootDirectory = foundation::Path("workspaces");
    bool showHelp = false;
};

struct InvestigationShowOptions
{
    std::string investigationId;
    foundation::Path rootDirectory = foundation::Path("workspaces");
    bool showHelp = false;
};

struct InvestigationOpenOptions
{
    std::string investigationId;
    std::string artifactId;
    foundation::Path rootDirectory = foundation::Path("workspaces");
    bool showHelp = false;
};

enum class InvestigationTimelineFormat
{
    Table,
    Json
};

struct InvestigationTimelineOptions
{
    std::string investigationId;
    foundation::Path rootDirectory = foundation::Path("workspaces");
    InvestigationTimelineFormat format = InvestigationTimelineFormat::Table;
    std::optional<std::size_t> limit;
    scope::workspace::TimelineSortOrder order = scope::workspace::TimelineSortOrder::Ascending;
    bool showHelp = false;
};

/**
 * @brief Parsed CLI invocation.
 */
struct ParsedCli
{
    CliCommand command = CliCommand::Help;
    AnalyzeOptions analyze;
    InvestigateOptions investigate;
    InvestigateOptions search;
    InvestigateOptions query;
    AnalyticsOptions analytics;
    ConfigValidateOptions configValidate;
    ExtensionsListOptions extensionsList;
    ExtensionsDescribeOptions extensionsDescribe;
    SessionSaveOptions sessionSave;
    SessionLoadOptions sessionLoad;
    SessionListOptions sessionList;
    InvestigationCreateOptions investigationCreate;
    InvestigationAddOptions investigationAdd;
    InvestigationAddNoteOptions investigationAddNote;
    InvestigationListOptions investigationList;
    InvestigationShowOptions investigationShow;
    InvestigationOpenOptions investigationOpen;
    InvestigationTimelineOptions investigationTimeline;
    AgentInvestigateOptions agentInvestigate;
    bool showGlobalHelp = false;
};

/**
 * @brief Parses command-line arguments.
 */
[[nodiscard]] std::optional<ParsedCli> parseCliArguments(int argc, char* argv[]);

} // namespace scope::cli
