/**
 * @file investigation_command.cpp
 * @brief Investigation container CLI subcommand implementation.
 */

#include "investigation_command.hpp"

#include "foundation/uuid.hpp"
#include "output_format.hpp"
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

} // namespace scope::cli
