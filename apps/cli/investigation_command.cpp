/**
 * @file investigation_command.cpp
 * @brief Investigation container CLI subcommand implementation.
 */

#include "investigation_command.hpp"

#include "foundation/uuid.hpp"
#include "output_format.hpp"
#include "crash_analyzer.hpp"
#include "crash_report.hpp"
#include "timeline_event.hpp"
#include "workspace.hpp"

#include <filesystem>
#include <iostream>
#include <regex>
#include <sstream>

namespace scope::cli
{

namespace
{

using scope::foundation::Path;
using scope::workspace::ArtifactIngestRequest;
using scope::workspace::ArtifactSource;
using scope::workspace::Investigation;
using scope::workspace::InvestigationCreateRequest;
using scope::workspace::CrashReport;
using scope::workspace::CrashThread;
using scope::workspace::TimelineEvent;
using scope::workspace::TimelineProjectionOptions;
using scope::workspace::TimelineProjectionResult;

bool isValidInvestigationId(const std::string& investigationId)
{
    static const std::regex pattern("^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$");

    return std::regex_match(investigationId, pattern);
}

Path investigationDirectory(const Path& rootDirectory, const std::string& investigationId)
{
    return Path(rootDirectory.string() + "/" + investigationId);
}

std::string inferArtifactType(const std::string& explicitType, const Path& sourceFile)
{
    if (!explicitType.empty())
    {
        return explicitType;
    }

    const std::string filename = sourceFile.filename().string();
    const std::size_t dot = filename.rfind('.');

    if (dot != std::string::npos)
    {
        const std::string extension = filename.substr(dot);

        if (extension == ".core")
        {
            return "core";
        }

        if (extension == ".pstack")
        {
            return "pstack";
        }
    }

    return "log";
}

void printManifestSummary(const scope::workspace::InvestigationManifest& manifest, std::ostream& output)
{
    output << "id: " << manifest.id << '\n'
           << "name: " << manifest.name << '\n'
           << "description: " << manifest.description << '\n'
           << "updatedAt: " << manifest.updatedAt << '\n'
           << "artifacts: " << manifest.artifacts.size() << '\n'
           << "primaryArtifactId: " << manifest.primaryArtifactId << '\n';
}

std::string escapeJsonString(const std::string& value)
{
    std::string escaped;
    escaped.reserve(value.size());

    for (const char character : value)
    {
        switch (character)
        {
        case '"':
            escaped += "\\\"";
            break;
        case '\\':
            escaped += "\\\\";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\t':
            escaped += "\\t";
            break;
        default:
            escaped.push_back(character);
            break;
        }
    }

    return escaped;
}

std::string formatEventSourceJson(const scope::workspace::EventSource& source)
{
    std::ostringstream output;
    output << "{\n"
           << "  \"artifactId\": \"" << escapeJsonString(source.artifactId) << "\",\n"
           << "  \"artifactType\": \"" << escapeJsonString(source.artifactType) << "\",\n"
           << "  \"artifactName\": \"" << escapeJsonString(source.artifactName) << "\"";

    if (source.lineNumber.has_value())
    {
        output << ",\n  \"lineNumber\": " << *source.lineNumber;
    }

    if (source.byteOffset.has_value())
    {
        output << ",\n  \"byteOffset\": " << *source.byteOffset;
    }

    output << "\n}";

    return output.str();
}

std::string formatStringMapJson(const std::map<std::string, std::string>& values)
{
    std::ostringstream output;
    output << '{';

    bool first = true;

    for (const auto& [key, value] : values)
    {
        if (!first)
        {
            output << ',';
        }

        first = false;
        output << "\n    \"" << escapeJsonString(key) << "\": \"" << escapeJsonString(value) << '"';
    }

    if (!values.empty())
    {
        output << '\n';
    }

    output << "  }";

    return output.str();
}

std::string formatTimelineEventJson(const TimelineEvent& event)
{
    std::ostringstream output;
    output << "{\n"
           << "  \"id\": \"" << escapeJsonString(event.id) << "\",\n"
           << "  \"timestamp\": \"" << escapeJsonString(event.timestamp) << "\",\n"
           << "  \"artifactId\": \"" << escapeJsonString(event.artifactId) << "\",\n"
           << "  \"eventType\": \"" << escapeJsonString(event.eventType) << "\",\n"
           << "  \"message\": \"" << escapeJsonString(event.message) << "\",\n"
           << "  \"source\": " << formatEventSourceJson(event.source) << ",\n"
           << "  \"metadata\": " << formatStringMapJson(event.metadata);

    if (event.severity.has_value())
    {
        output << ",\n  \"severity\": \"" << escapeJsonString(*event.severity) << '"';
    }

    output << "\n}";

    return output.str();
}

std::string formatInvestigationTimelineJson(const std::string& investigationId, const TimelineProjectionResult& result)
{
    std::ostringstream output;
    output << "{\n"
           << "  \"investigationId\": \"" << escapeJsonString(investigationId) << "\",\n"
           << "  \"events\": [";

    for (std::size_t index = 0U; index < result.events.size(); ++index)
    {
        if (index > 0U)
        {
            output << ',';
        }

        output << "\n    " << formatTimelineEventJson(result.events[index]);
    }

    output << "\n  ],\n"
           << "  \"pagination\": {\n"
           << "    \"truncated\": " << (result.truncated ? "true" : "false");

    if (result.totalMatched.has_value())
    {
        output << ",\n    \"totalMatched\": " << *result.totalMatched;
    }

    output << "\n  },\n"
           << "  \"warnings\": [";

    for (std::size_t index = 0U; index < result.warnings.size(); ++index)
    {
        if (index > 0U)
        {
            output << ',';
        }

        output << "\n    \"" << escapeJsonString(result.warnings[index]) << '"';
    }

    output << "\n  ]\n}";

    return output.str();
}

std::string formatTimelineSourceLabel(const scope::workspace::EventSource& source)
{
    std::ostringstream output;
    output << source.artifactName;

    if (source.lineNumber.has_value())
    {
        output << ':' << *source.lineNumber;
    }

    return output.str();
}

void printInvestigationTimelineTable(const TimelineProjectionResult& result, std::ostream& output)
{
    output << "timestamp\teventType\tsource\tmessage\n";

    for (const TimelineEvent& event : result.events)
    {
        output << event.timestamp << '\t' << event.eventType << '\t' << formatTimelineSourceLabel(event.source) << '\t'
               << event.message << '\n';
    }

    if (result.truncated)
    {
        output << "\n(truncated";

        if (result.totalMatched.has_value())
        {
            output << ", total matched: " << *result.totalMatched;
        }

        output << ")\n";
    }

    for (const std::string& warning : result.warnings)
    {
        output << "warning: " << warning << '\n';
    }
}

std::string formatCrashReportJson(const CrashReport& report)
{
    std::ostringstream output;
    output << "{\n"
           << "  \"id\": \"" << escapeJsonString(report.id) << "\",\n"
           << "  \"artifactId\": \"" << escapeJsonString(report.artifactId) << "\",\n"
           << "  \"artifactType\": \"" << escapeJsonString(report.artifactType) << "\",\n"
           << "  \"status\": \""
           << escapeJsonString(scope::workspace::crashAnalysisStatusToString(report.status)) << "\",\n"
           << "  \"summary\": \"" << escapeJsonString(report.summary) << "\",\n"
           << "  \"threads\": [";

    for (std::size_t threadIndex = 0U; threadIndex < report.threads.size(); ++threadIndex)
    {
        if (threadIndex > 0U)
        {
            output << ',';
        }

        const CrashThread& thread = report.threads[threadIndex];
        output << "\n    {\n"
               << "      \"id\": \"" << escapeJsonString(thread.id) << "\",\n"
               << "      \"name\": \"" << escapeJsonString(thread.name) << "\",\n"
               << "      \"isFaultThread\": " << (thread.isFaultThread ? "true" : "false") << ",\n"
               << "      \"frames\": [";

        for (std::size_t frameIndex = 0U; frameIndex < thread.frames.size(); ++frameIndex)
        {
            if (frameIndex > 0U)
            {
                output << ',';
            }

            const auto& frame = thread.frames[frameIndex];
            output << "\n        {\n"
                   << "          \"index\": " << frame.index << ",\n"
                   << "          \"address\": \"" << escapeJsonString(frame.address) << "\",\n"
                   << "          \"symbol\": \"" << escapeJsonString(frame.symbol) << "\"";

            if (frame.location.has_value())
            {
                output << ",\n          \"location\": \"" << escapeJsonString(*frame.location) << '"';
            }

            output << "\n        }";
        }

        output << "\n      ]\n    }";
    }

    output << "\n  ],\n"
           << "  \"observations\": [";

    for (std::size_t index = 0U; index < report.observations.size(); ++index)
    {
        if (index > 0U)
        {
            output << ',';
        }

        output << "\n    \"" << escapeJsonString(report.observations[index]) << '"';
    }

    output << "\n  ],\n"
           << "  \"warnings\": [";

    for (std::size_t index = 0U; index < report.warnings.size(); ++index)
    {
        if (index > 0U)
        {
            output << ',';
        }

        output << "\n    \"" << escapeJsonString(report.warnings[index]) << '"';
    }

    output << "\n  ]";

    if (report.signal.has_value())
    {
        output << ",\n  \"signal\": \"" << escapeJsonString(*report.signal) << '"';
    }

    if (report.faultThreadId.has_value())
    {
        output << ",\n  \"faultThreadId\": \"" << escapeJsonString(*report.faultThreadId) << '"';
    }

    output << "\n}";

    return output.str();
}

void printInvestigationCrashTable(const CrashReport& report, std::ostream& output)
{
    output << "status\tsummary\n" << scope::workspace::crashAnalysisStatusToString(report.status) << '\t'
           << report.summary << '\n';

    if (report.signal.has_value())
    {
        output << "signal: " << *report.signal << '\n';
    }

    for (const CrashThread& thread : report.threads)
    {
        output << "\n[" << thread.name << (thread.isFaultThread ? " *fault*" : "") << "]\n";

        for (const auto& frame : thread.frames)
        {
            output << "  #" << frame.index << ' ' << frame.address << ' ' << frame.symbol;

            if (frame.location.has_value())
            {
                output << " at " << *frame.location;
            }

            output << '\n';
        }
    }

    for (const std::string& observation : report.observations)
    {
        output << "observation: " << observation << '\n';
    }

    for (const std::string& warning : report.warnings)
    {
        output << "warning: " << warning << '\n';
    }
}

std::optional<std::string> resolveCrashArtifactId(const Investigation& investigation, const std::string& explicitId)
{
    if (!explicitId.empty())
    {
        return explicitId;
    }

    for (const auto& artifact : investigation.manifest().artifacts)
    {
        if (scope::workspace::isCrashAnalyzableArtifactType(artifact.type))
        {
            return artifact.id;
        }
    }

    return std::nullopt;
}

} // namespace

void printInvestigationCreateUsage(std::ostream& output)
{
    output << "Usage: logscope investigation create --name <name> [--description <text>] [--dir <root>]\n"
           << "\n"
           << "Options:\n"
           << "  --name <name>         Investigation name (required)\n"
           << "  --description <text>  Optional description\n"
           << "  --dir <root>          Investigations root directory (default: ./workspaces)\n"
           << "  --help, -h            Show this help message\n";
}

void printInvestigationAddUsage(std::ostream& output)
{
    output << "Usage: logscope investigation add <investigation-id> <source> [--type log|pstack|core] "
              "[--display-name <name>] [--role <string>] [--dir <root>]\n"
           << "\n"
           << "Options:\n"
           << "  --type <type>         Artifact type (default: infer from extension, else log)\n"
           << "  --display-name <name> Display name for the imported file\n"
           << "  --role <string>       Optional metadata role (e.g. application, system)\n"
           << "  --dir <root>          Investigations root directory (default: ./workspaces)\n"
           << "  --help, -h            Show this help message\n";
}

void printInvestigationAddNoteUsage(std::ostream& output)
{
    output << "Usage: logscope investigation add-note <investigation-id> --title <title> --body <text> [--dir <root>]\n"
           << "\n"
           << "Options:\n"
           << "  --title <title>       Note title (required)\n"
           << "  --body <text>         Note body\n"
           << "  --dir <root>          Investigations root directory (default: ./workspaces)\n"
           << "  --help, -h            Show this help message\n";
}

void printInvestigationListUsage(std::ostream& output)
{
    output << "Usage: logscope investigation list [--dir <root>]\n"
           << "\n"
           << "Options:\n"
           << "  --dir <root>          Investigations root directory (default: ./workspaces)\n"
           << "  --help, -h            Show this help message\n";
}

void printInvestigationShowUsage(std::ostream& output)
{
    output << "Usage: logscope investigation show <investigation-id> [--dir <root>]\n"
           << "\n"
           << "Options:\n"
           << "  --dir <root>          Investigations root directory (default: ./workspaces)\n"
           << "  --help, -h            Show this help message\n";
}

void printInvestigationOpenUsage(std::ostream& output)
{
    output << "Usage: logscope investigation open <investigation-id> [--artifact <id>] [--dir <root>]\n"
           << "\n"
           << "Prints the resolved path for a log artifact (entry artifact by default).\n"
           << "\n"
           << "Options:\n"
           << "  --artifact <id>       Open a specific log artifact instead of the entry artifact\n"
           << "  --dir <root>          Investigations root directory (default: ./workspaces)\n"
           << "  --help, -h            Show this help message\n";
}

void printInvestigationTimelineUsage(std::ostream& output)
{
    output << "Usage: logscope investigation timeline <investigation-id> [--format json|table] [--limit N] "
              "[--order asc|desc] [--dir <root>]\n"
           << "\n"
           << "Options:\n"
           << "  --format <format>     Output format: table or json (default: table)\n"
           << "  --limit <N>           Maximum number of timeline events to return\n"
           << "  --order <order>       Sort order: asc or desc (default: asc)\n"
           << "  --dir <root>          Investigations root directory (default: ./workspaces)\n"
           << "  --help, -h            Show this help message\n";
}

void printInvestigationCrashUsage(std::ostream& output)
{
    output << "Usage: logscope investigation crash <investigation-id> [--artifact <artifact-id>] "
              "[--format json|table] [--dir <root>]\n"
           << "\n"
           << "Options:\n"
           << "  --artifact <id>       Artifact to analyze (default: first pstack or core)\n"
           << "  --format <format>     Output format: table or json (default: table)\n"
           << "  --dir <root>          Investigations root directory (default: ./workspaces)\n"
           << "  --help, -h            Show this help message\n";
}

void printInvestigationLinksUsage(std::ostream& output)
{
    output << "Usage:\n"
           << "  logscope investigation links list <investigation-id> [--format json|table] [--dir <root>]\n"
           << "  logscope investigation links add <investigation-id> --source <event-id> --target <event-id> "
              "[--type RELATED|PRECEDES|FOLLOWS|SUPPORTS] [--note \"...\"] [--dir <root>]\n"
           << "  logscope investigation links remove <investigation-id> --link <link-id> [--dir <root>]\n"
           << "\n"
           << "Options:\n"
           << "  --format <format>     Output format for list: table or json (default: table)\n"
           << "  --source <event-id>   Source timeline event id (add)\n"
           << "  --target <event-id>   Target timeline event id (add)\n"
           << "  --type <type>         Link type (default: RELATED)\n"
           << "  --note <text>         Optional investigator note (add)\n"
           << "  --link <link-id>      Evidence link id to remove\n"
           << "  --dir <root>          Investigations root directory (default: ./workspaces)\n"
           << "  --help, -h            Show this help message\n";
}

int runInvestigationCreateCommand(const InvestigationCreateOptions& options,
                                  std::ostream& output,
                                  std::ostream& errorOutput)
{
    if (options.showHelp)
    {
        printInvestigationCreateUsage(output);

        return 0;
    }

    if (options.name.empty())
    {
        errorOutput << "Error: --name is required.\n";
        printInvestigationCreateUsage(errorOutput);

        return 1;
    }

    std::error_code errorCode;
    std::filesystem::create_directories(options.rootDirectory.string(), errorCode);

    const Path stagingDir =
        Path(options.rootDirectory.string() + "/.staging-" + scope::foundation::Uuid::generate().toString());

    InvestigationCreateRequest request;
    request.name = options.name;
    request.description = options.description;

    const auto createResult = Investigation::create(stagingDir, request);

    if (!createResult)
    {
        errorOutput << createResult.error().message() << '\n';

        return 1;
    }

    const std::string investigationId = createResult->manifest().id;
    const Path finalDir = investigationDirectory(options.rootDirectory, investigationId);
    std::filesystem::rename(stagingDir.string(), finalDir.string(), errorCode);

    if (errorCode)
    {
        std::filesystem::remove_all(stagingDir.string(), errorCode);
        errorOutput << "Failed to finalize investigation directory.\n";

        return 1;
    }

    output << investigationId << '\n';

    return 0;
}

int runInvestigationAddCommand(const InvestigationAddOptions& options,
                               std::ostream& output,
                               std::ostream& errorOutput)
{
    if (options.showHelp)
    {
        printInvestigationAddUsage(output);

        return 0;
    }

    if (!isValidInvestigationId(options.investigationId))
    {
        errorOutput << "Invalid investigation id.\n";

        return 1;
    }

    const Path investigationDir = investigationDirectory(options.rootDirectory, options.investigationId);
    auto investigationResult = Investigation::open(investigationDir);

    if (!investigationResult)
    {
        errorOutput << investigationResult.error().message() << '\n';

        return 1;
    }

    Investigation investigation = std::move(*investigationResult);

    const std::string artifactType = inferArtifactType(options.artifactType, options.logFile);

    if (artifactType != "log" && artifactType != "pstack" && artifactType != "core")
    {
        errorOutput << "Unsupported artifact type: " << artifactType << '\n';

        return 1;
    }

    ArtifactIngestRequest request;
    request.type = artifactType;
    request.name = options.displayName.empty() ? options.logFile.string() : options.displayName;
    request.sourceFile = options.logFile;
    request.source = ArtifactSource{"upload", request.name};
    request.role = options.role;

    const auto artifactResult = investigation.addArtifact(request);

    if (!artifactResult)
    {
        errorOutput << artifactResult.error().message() << '\n';

        return 1;
    }

    if (!investigation.persist())
    {
        errorOutput << "Failed to persist investigation manifest.\n";

        return 1;
    }

    output << artifactResult->id << '\n';

    return 0;
}

int runInvestigationAddNoteCommand(const InvestigationAddNoteOptions& options,
                                   std::ostream& output,
                                   std::ostream& errorOutput)
{
    if (options.showHelp)
    {
        printInvestigationAddNoteUsage(output);

        return 0;
    }

    if (!isValidInvestigationId(options.investigationId))
    {
        errorOutput << "Invalid investigation id.\n";

        return 1;
    }

    if (options.title.empty())
    {
        errorOutput << "Error: --title is required.\n";

        return 1;
    }

    const Path investigationDir = investigationDirectory(options.rootDirectory, options.investigationId);
    auto investigationResult = Investigation::open(investigationDir);

    if (!investigationResult)
    {
        errorOutput << investigationResult.error().message() << '\n';

        return 1;
    }

    Investigation investigation = std::move(*investigationResult);

    ArtifactIngestRequest request;
    request.type = "note";
    request.name = options.title;
    request.noteBody = options.body;
    request.source = ArtifactSource{"inline", options.title};

    const auto artifactResult = investigation.addArtifact(request);

    if (!artifactResult)
    {
        errorOutput << artifactResult.error().message() << '\n';

        return 1;
    }

    if (!investigation.persist())
    {
        errorOutput << "Failed to persist investigation manifest.\n";

        return 1;
    }

    output << artifactResult->id << '\n';

    return 0;
}

int runInvestigationListCommand(const InvestigationListOptions& options,
                                std::ostream& output,
                                std::ostream& errorOutput)
{
    if (options.showHelp)
    {
        printInvestigationListUsage(output);

        return 0;
    }

    std::error_code errorCode;

    if (!std::filesystem::is_directory(options.rootDirectory.string(), errorCode))
    {
        return 0;
    }

    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator(options.rootDirectory.string(), errorCode))
    {
        if (!entry.is_directory())
        {
            continue;
        }

        const std::string directoryName = entry.path().filename().string();

        if (!isValidInvestigationId(directoryName))
        {
            continue;
        }

        const auto investigationResult = Investigation::open(Path(entry.path().string()));

        if (!investigationResult)
        {
            continue;
        }

        const auto& manifest = investigationResult->manifest();
        output << manifest.id << '\t' << manifest.name << '\t' << manifest.updatedAt << '\t'
               << manifest.artifacts.size() << '\n';
    }

    return 0;
}

int runInvestigationShowCommand(const InvestigationShowOptions& options,
                                std::ostream& output,
                                std::ostream& errorOutput)
{
    if (options.showHelp)
    {
        printInvestigationShowUsage(output);

        return 0;
    }

    if (!isValidInvestigationId(options.investigationId))
    {
        errorOutput << "Invalid investigation id.\n";

        return 1;
    }

    const Path investigationDir = investigationDirectory(options.rootDirectory, options.investigationId);
    const auto investigationResult = Investigation::open(investigationDir);

    if (!investigationResult)
    {
        errorOutput << investigationResult.error().message() << '\n';

        return 1;
    }

    const auto& manifest = investigationResult->manifest();
    printManifestSummary(manifest, output);

    for (const auto& artifact : manifest.artifacts)
    {
        const bool isEntry = artifact.id == manifest.primaryArtifactId;
        output << "  - " << artifact.id << " (" << artifact.type << ")";

        if (isEntry)
        {
            output << " [entry]";
        }

        if (!artifact.metadata.empty())
        {
            const auto roleIterator = artifact.metadata.find("role");

            if (roleIterator != artifact.metadata.end())
            {
                output << " role=" << roleIterator->second;
            }
        }

        output << ' ' << artifact.name << '\n';
    }

    return 0;
}

int runInvestigationOpenCommand(const InvestigationOpenOptions& options,
                                std::ostream& output,
                                std::ostream& errorOutput)
{
    if (options.showHelp)
    {
        printInvestigationOpenUsage(output);

        return 0;
    }

    if (!isValidInvestigationId(options.investigationId))
    {
        errorOutput << "Invalid investigation id.\n";

        return 1;
    }

    const Path investigationDir = investigationDirectory(options.rootDirectory, options.investigationId);
    const auto investigationResult = Investigation::open(investigationDir);

    if (!investigationResult)
    {
        errorOutput << investigationResult.error().message() << '\n';

        return 1;
    }

    const auto entryArtifactResult = options.artifactId.empty()
                                         ? investigationResult->entryArtifact()
                                         : investigationResult->artifactById(options.artifactId);

    if (!entryArtifactResult)
    {
        errorOutput << entryArtifactResult.error().message() << '\n';

        return 1;
    }

    output << entryArtifactResult->type << '\t' << entryArtifactResult->name << '\n';

    if (entryArtifactResult->type == "log")
    {
        const auto dataPathResult = options.artifactId.empty()
                                        ? investigationResult->entryArtifactDataPath()
                                        : investigationResult->logArtifactDataPath(options.artifactId);

        if (!dataPathResult)
        {
            errorOutput << dataPathResult.error().message() << '\n';

            return 1;
        }

        output << dataPathResult->string() << '\n';
    }

    return 0;
}

int runInvestigationTimelineCommand(const InvestigationTimelineOptions& options,
                                    std::ostream& output,
                                    std::ostream& errorOutput)
{
    if (options.showHelp)
    {
        printInvestigationTimelineUsage(output);

        return 0;
    }

    if (!isValidInvestigationId(options.investigationId))
    {
        errorOutput << "Invalid investigation id.\n";

        return 1;
    }

    const Path investigationDir = investigationDirectory(options.rootDirectory, options.investigationId);
    const auto investigationResult = Investigation::open(investigationDir);

    if (!investigationResult)
    {
        errorOutput << investigationResult.error().message() << '\n';

        return 1;
    }

    TimelineProjectionOptions projectionOptions;
    projectionOptions.order = options.order;

    if (options.limit.has_value())
    {
        projectionOptions.limit = *options.limit;
    }

    const auto timelineResult = investigationResult->projectTimeline(projectionOptions);

    if (!timelineResult)
    {
        errorOutput << timelineResult.error().message() << '\n';

        return 1;
    }

    if (options.format == InvestigationTimelineFormat::Json)
    {
        output << formatInvestigationTimelineJson(options.investigationId, *timelineResult) << '\n';
    }
    else
    {
        printInvestigationTimelineTable(*timelineResult, output);
    }

    return 0;
}

int runInvestigationCrashCommand(const InvestigationCrashOptions& options,
                                 std::ostream& output,
                                 std::ostream& errorOutput)
{
    if (options.showHelp)
    {
        printInvestigationCrashUsage(output);

        return 0;
    }

    if (!isValidInvestigationId(options.investigationId))
    {
        errorOutput << "Invalid investigation id.\n";

        return 1;
    }

    const Path investigationDir = investigationDirectory(options.rootDirectory, options.investigationId);
    const auto investigationResult = Investigation::open(investigationDir);

    if (!investigationResult)
    {
        errorOutput << investigationResult.error().message() << '\n';

        return 1;
    }

    const auto artifactId = resolveCrashArtifactId(*investigationResult, options.artifactId);

    if (!artifactId.has_value())
    {
        errorOutput << "No analyzable crash artifact found. Add a pstack or core artifact.\n";

        return 1;
    }

    const auto crashResult = investigationResult->analyzeCrash(*artifactId);

    if (!crashResult)
    {
        errorOutput << crashResult.error().message() << '\n';

        return 1;
    }

    if (crashResult->status == scope::workspace::CrashAnalysisStatus::NotSupported)
    {
        errorOutput << "Artifact does not support crash analysis.\n";

        if (options.format == InvestigationTimelineFormat::Json)
        {
            output << formatCrashReportJson(*crashResult) << '\n';
        }

        return 1;
    }

    if (options.format == InvestigationTimelineFormat::Json)
    {
        output << formatCrashReportJson(*crashResult) << '\n';
    }
    else
    {
        printInvestigationCrashTable(*crashResult, output);
    }

    return 0;
}

std::string formatEvidenceLinkRecordJson(const scope::workspace::EvidenceLinkRecord& link)
{
    std::ostringstream output;
    output << "{\n"
           << "        \"id\": \"" << escapeJsonString(link.id) << "\",\n"
           << "        \"type\": \"" << escapeJsonString(scope::workspace::evidenceLinkTypeToString(link.type))
           << "\",\n"
           << "        \"source\": {\n"
           << "          \"kind\": \"" << escapeJsonString(link.source.kind) << "\",\n"
           << "          \"eventId\": \"" << escapeJsonString(link.source.eventId) << "\"\n"
           << "        },\n"
           << "        \"target\": {\n"
           << "          \"kind\": \"" << escapeJsonString(link.target.kind) << "\",\n"
           << "          \"eventId\": \"" << escapeJsonString(link.target.eventId) << "\"\n"
           << "        },\n"
           << "        \"createdAt\": \"" << escapeJsonString(link.createdAt) << "\",\n"
           << "        \"status\": \"" << escapeJsonString(scope::workspace::evidenceLinkStatusToString(link.status))
           << "\"";

    if (link.note.has_value())
    {
        output << ",\n        \"note\": \"" << escapeJsonString(*link.note) << '"';
    }
    else
    {
        output << ",\n        \"note\": null";
    }

    output << "\n      }";

    return output.str();
}

std::string formatEvidenceLinksListJson(const std::string& investigationId,
                                        const std::vector<scope::workspace::EvidenceLinkRecord>& links)
{
    std::ostringstream output;
    output << "{\n"
           << "  \"investigationId\": \"" << escapeJsonString(investigationId) << "\",\n"
           << "  \"links\": [";

    for (std::size_t index = 0U; index < links.size(); ++index)
    {
        if (index > 0U)
        {
            output << ',';
        }

        output << "\n      " << formatEvidenceLinkRecordJson(links[index]);
    }

    if (!links.empty())
    {
        output << '\n';
    }

    output << "    ]\n}";

    return output.str();
}

void printEvidenceLinksTable(const std::vector<scope::workspace::EvidenceLinkRecord>& links, std::ostream& output)
{
    output << "id\ttype\tsource\ttarget\tstatus\tnote\n";

    for (const scope::workspace::EvidenceLinkRecord& link : links)
    {
        std::string noteColumn;

        if (link.note.has_value())
        {
            noteColumn = *link.note;

            if (noteColumn.size() > 60U)
            {
                noteColumn.resize(57U);
                noteColumn += "...";
            }
        }

        output << link.id << '\t' << scope::workspace::evidenceLinkTypeToString(link.type) << '\t'
               << link.source.eventId << '\t' << link.target.eventId << '\t'
               << scope::workspace::evidenceLinkStatusToString(link.status) << '\t' << noteColumn << '\n';
    }
}

int runInvestigationLinksCommand(const InvestigationLinksOptions& options,
                                 std::ostream& output,
                                 std::ostream& errorOutput)
{
    if (options.showHelp)
    {
        printInvestigationLinksUsage(output);

        return 0;
    }

    if (!isValidInvestigationId(options.investigationId))
    {
        errorOutput << "Invalid investigation id.\n";

        return 1;
    }

    const Path investigationDir = investigationDirectory(options.rootDirectory, options.investigationId);
    const auto investigationResult = Investigation::open(investigationDir);

    if (!investigationResult)
    {
        errorOutput << investigationResult.error().message() << '\n';

        return 2;
    }

    Investigation investigation = std::move(*investigationResult);

    if (options.subcommand == InvestigationLinksSubcommand::List)
    {
        const auto linksResult = investigation.listEvidenceLinks();

        if (!linksResult)
        {
            errorOutput << linksResult.error().message() << '\n';

            return 2;
        }

        if (options.format == InvestigationTimelineFormat::Json)
        {
            output << formatEvidenceLinksListJson(options.investigationId, *linksResult) << '\n';
        }
        else
        {
            printEvidenceLinksTable(*linksResult, output);
        }

        return 0;
    }

    if (options.subcommand == InvestigationLinksSubcommand::Add)
    {
        if (options.sourceEventId.empty() || options.targetEventId.empty())
        {
            errorOutput << "Error: --source and --target are required.\n";
            printInvestigationLinksUsage(errorOutput);

            return 1;
        }

        const std::optional<scope::workspace::EvidenceLinkType> parsedType =
            scope::workspace::parseEvidenceLinkType(options.linkType);

        if (!parsedType)
        {
            errorOutput << "Invalid link type.\n";

            return 1;
        }

        scope::workspace::EvidenceLinkCreateRequest createRequest;
        createRequest.type = *parsedType;
        createRequest.source.kind = "timeline_event";
        createRequest.source.eventId = options.sourceEventId;
        createRequest.target.kind = "timeline_event";
        createRequest.target.eventId = options.targetEventId;

        if (!options.note.empty())
        {
            createRequest.note = options.note;
        }

        const auto linkResult = investigation.addEvidenceLink(createRequest);

        if (!linkResult)
        {
            errorOutput << linkResult.error().message() << '\n';

            return 2;
        }

        output << formatEvidenceLinkRecordJson(*linkResult) << '\n';

        return 0;
    }

    if (options.linkId.empty())
    {
        errorOutput << "Error: --link is required.\n";
        printInvestigationLinksUsage(errorOutput);

        return 1;
    }

    const auto removeResult = investigation.removeEvidenceLink(options.linkId);

    if (!removeResult)
    {
        errorOutput << removeResult.error().message() << '\n';

        return 2;
    }

    return 0;
}

} // namespace scope::cli
