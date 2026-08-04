/**
 * @file workspace_store.cpp
 */

#include "workspace_store.hpp"

#include "json_parse.hpp"
#include "rest_json.hpp"

#include "foundation/clock.hpp"
#include "foundation/uuid.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>

namespace scope::web
{

namespace
{

std::string currentTimestampIso()
{
    return foundation::Clock::now().toString();
}

std::string extractJsonString(const std::string& object, const std::string_view key)
{
    const std::optional<std::string> value = jsonStringField(object, key);

    return value.value_or(std::string());
}

WorkspaceSourceRef parseSourceRef(const std::string& object)
{
    WorkspaceSourceRef ref;
    ref.type = extractJsonString(object, "type");

    if (ref.type.empty())
    {
        ref.type = "upload";
    }

    ref.displayName = extractJsonString(object, "displayName");
    ref.path = extractJsonString(object, "path");

    return ref;
}

WorkspaceSummary parseSummary(const std::string& object)
{
    WorkspaceSummary summary;

    const std::size_t key = object.find("\"summary\"");

    if (key == std::string::npos)
    {
        return summary;
    }

    const std::size_t openBrace = object.find('{', key);

    if (openBrace == std::string::npos)
    {
        return summary;
    }

    std::size_t depth = 0U;
    std::size_t closeBrace = openBrace;

    for (std::size_t index = openBrace; index < object.size(); ++index)
    {
        if (object[index] == '{')
        {
            ++depth;
        }
        else if (object[index] == '}')
        {
            --depth;

            if (depth == 0U)
            {
                closeBrace = index;
                break;
            }
        }
    }

    const std::string summaryObject = object.substr(openBrace, closeBrace - openBrace + 1U);

    if (const std::optional<bool> hasModel = jsonBoolField(summaryObject, "hasModel"))
    {
        summary.hasModel = *hasModel;
    }

    if (const std::optional<std::int64_t> lineCount = jsonIntField(summaryObject, "lineCount"))
    {
        summary.lineCount = static_cast<std::uint64_t>(*lineCount);
    }

    if (const std::optional<std::int64_t> errorCount = jsonIntField(summaryObject, "errorCount"))
    {
        summary.errorCount = static_cast<std::uint64_t>(*errorCount);
    }

    return summary;
}

std::string formatSourceRefJson(const WorkspaceSourceRef& ref)
{
    std::ostringstream output;
    output << "{\n"
           << "    \"type\": \"" << escapeJsonString(ref.type) << "\",\n"
           << "    \"displayName\": \"" << escapeJsonString(ref.displayName) << "\"";

    if (!ref.path.empty())
    {
        output << ",\n    \"path\": \"" << escapeJsonString(ref.path) << '"';
    }

    output << "\n  }";

    return output.str();
}

std::string formatSummaryJson(const WorkspaceSummary& summary)
{
    std::ostringstream output;
    output << "{\n"
           << "    \"hasModel\": " << (summary.hasModel ? "true" : "false") << ",\n"
           << "    \"lineCount\": " << summary.lineCount << ",\n"
           << "    \"errorCount\": " << summary.errorCount << "\n"
           << "  }";

    return output.str();
}

std::string formatWorkspaceMetadataJson(const WorkspaceMetadata& metadata)
{
    std::ostringstream output;
    output << "{\n"
           << "  \"schemaVersion\": 1,\n"
           << "  \"id\": \"" << escapeJsonString(metadata.id) << "\",\n"
           << "  \"name\": \"" << escapeJsonString(metadata.name) << "\",\n"
           << "  \"description\": \"" << escapeJsonString(metadata.description) << "\",\n"
           << "  \"createdAt\": \"" << escapeJsonString(metadata.createdAt) << "\",\n"
           << "  \"updatedAt\": \"" << escapeJsonString(metadata.updatedAt) << "\",\n"
           << "  \"sourceRef\": " << formatSourceRefJson(metadata.sourceRef) << ",\n"
           << "  \"summary\": " << formatSummaryJson(metadata.summary) << ",\n"
           << "  \"snapshotFile\": \"" << escapeJsonString(metadata.snapshotFile) << "\"\n"
           << '}';

    return output.str();
}

} // namespace

WorkspaceStore::WorkspaceStore(const WebConfig& config)
{
    if (!config.workspaceDir.string().empty())
    {
        m_rootDirectory = config.workspaceDir;
    }
    else
    {
        m_rootDirectory = foundation::Path((std::filesystem::current_path() / "workspaces").string());
    }

    std::error_code errorCode;
    std::filesystem::create_directories(m_rootDirectory.string(), errorCode);
}

const foundation::Path& WorkspaceStore::rootDirectory() const noexcept
{
    return m_rootDirectory;
}

bool WorkspaceStore::isValidWorkspaceId(const std::string& workspaceId)
{
    static const std::regex pattern("^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$");

    if (!std::regex_match(workspaceId, pattern))
    {
        return false;
    }

    return static_cast<bool>(foundation::Uuid::parse(workspaceId));
}

foundation::Path WorkspaceStore::workspaceDirectory(const std::string& workspaceId) const
{
    return foundation::Path(m_rootDirectory.string() + "/" + workspaceId);
}

foundation::Result<bool> WorkspaceStore::ensureUnderRoot(const foundation::Path& path) const
{
    std::error_code errorCode;
    const std::filesystem::path absolutePath = std::filesystem::weakly_canonical(path.string(), errorCode);
    const std::filesystem::path absoluteRoot = std::filesystem::weakly_canonical(m_rootDirectory.string(), errorCode);

    if (errorCode)
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Invalid workspace path."));
    }

    const std::string absoluteString = absolutePath.string();
    const std::string rootString = absoluteRoot.string();

    if (absoluteString.rfind(rootString, 0) != 0)
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Path escapes workspace directory."));
    }

    return foundation::Result<bool>(true);
}

foundation::Result<WorkspaceMetadata> WorkspaceStore::loadMetadata(const foundation::Path& workspaceDir) const
{
    const foundation::Path metadataFile = foundation::Path(workspaceDir.string() + "/workspace.json");

    std::ifstream stream(metadataFile.string());

    if (!stream)
    {
        return foundation::Result<WorkspaceMetadata>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Workspace not found."));
    }

    std::ostringstream buffer;
    buffer << stream.rdbuf();
    const std::string content = buffer.str();

    WorkspaceMetadata metadata;
    metadata.id = extractJsonString(content, "id");
    metadata.name = extractJsonString(content, "name");
    metadata.description = extractJsonString(content, "description");
    metadata.createdAt = extractJsonString(content, "createdAt");
    metadata.updatedAt = extractJsonString(content, "updatedAt");
    metadata.snapshotFile = extractJsonString(content, "snapshotFile");

    if (metadata.snapshotFile.empty())
    {
        metadata.snapshotFile = "snapshot.session";
    }

    const std::size_t sourceRefKey = content.find("\"sourceRef\"");

    if (sourceRefKey != std::string::npos)
    {
        const std::size_t openBrace = content.find('{', sourceRefKey);

        if (openBrace != std::string::npos)
        {
            std::size_t depth = 0U;
            std::size_t closeBrace = openBrace;

            for (std::size_t index = openBrace; index < content.size(); ++index)
            {
                if (content[index] == '{')
                {
                    ++depth;
                }
                else if (content[index] == '}')
                {
                    --depth;

                    if (depth == 0U)
                    {
                        closeBrace = index;
                        break;
                    }
                }
            }

            metadata.sourceRef = parseSourceRef(content.substr(openBrace, closeBrace - openBrace + 1U));
        }
    }

    metadata.summary = parseSummary(content);

    const std::string directoryName = std::filesystem::path(workspaceDir.string()).filename().string();

    if (!metadata.id.empty() && metadata.id != directoryName)
    {
        return foundation::Result<WorkspaceMetadata>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Workspace id mismatch."));
    }

    if (metadata.id.empty())
    {
        metadata.id = directoryName;
    }

    return foundation::Result<WorkspaceMetadata>(std::move(metadata));
}

foundation::Result<bool> WorkspaceStore::saveMetadata(const foundation::Path& workspaceDir,
                                                    const WorkspaceMetadata& metadata) const
{
    const foundation::Path metadataFile = foundation::Path(workspaceDir.string() + "/workspace.json");
    const foundation::Path tempFile = foundation::Path(workspaceDir.string() + "/workspace.json.tmp");
    const std::string json = formatWorkspaceMetadataJson(metadata);

    {
        std::ofstream stream(tempFile.string(), std::ios::binary | std::ios::trunc);

        if (!stream)
        {
            return foundation::Result<bool>(
                foundation::Error(foundation::ErrorCode::IOError, "Failed to write workspace metadata."));
        }

        stream << json;
    }

    std::error_code errorCode;
    std::filesystem::rename(tempFile.string(), metadataFile.string(), errorCode);

    if (errorCode)
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::IOError, "Failed to finalize workspace metadata."));
    }

    return foundation::Result<bool>(true);
}

foundation::Result<WorkspaceMetadata> WorkspaceStore::create(const WorkspaceCreateRequest& request)
{
    if (request.name.empty() || request.name.size() > 256U)
    {
        return foundation::Result<WorkspaceMetadata>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Workspace name is required (max 256 characters)."));
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    const std::string workspaceId = foundation::Uuid::generate().toString();
    const foundation::Path workspaceDir = workspaceDirectory(workspaceId);

    std::error_code errorCode;
    std::filesystem::create_directories(workspaceDir.string(), errorCode);

    if (errorCode)
    {
        return foundation::Result<WorkspaceMetadata>(
            foundation::Error(foundation::ErrorCode::IOError, "Failed to create workspace directory."));
    }

    const std::string timestamp = currentTimestampIso();
    WorkspaceMetadata metadata;
    metadata.id = workspaceId;
    metadata.name = request.name;
    metadata.description = request.description;
    metadata.createdAt = timestamp;
    metadata.updatedAt = timestamp;

    if (request.sourceRef.has_value())
    {
        metadata.sourceRef = *request.sourceRef;
    }

    const auto saveResult = saveMetadata(workspaceDir, metadata);

    if (!saveResult)
    {
        return foundation::Result<WorkspaceMetadata>(saveResult.error());
    }

    return foundation::Result<WorkspaceMetadata>(std::move(metadata));
}

foundation::Result<WorkspaceListResult> WorkspaceStore::list(const int limit) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    WorkspaceListResult result;
    std::vector<WorkspaceMetadata> all;

    std::error_code errorCode;

    if (!std::filesystem::is_directory(m_rootDirectory.string(), errorCode))
    {
        return foundation::Result<WorkspaceListResult>(std::move(result));
    }

    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator(m_rootDirectory.string(), errorCode))
    {
        if (!entry.is_directory())
        {
            continue;
        }

        const std::string directoryName = entry.path().filename().string();

        if (!isValidWorkspaceId(directoryName))
        {
            continue;
        }

        const auto metadataResult = loadMetadata(foundation::Path(entry.path().string()));

        if (!metadataResult)
        {
            continue;
        }

        all.push_back(*metadataResult);
    }

    std::sort(all.begin(), all.end(), [](const WorkspaceMetadata& left, const WorkspaceMetadata& right) {
        return left.updatedAt > right.updatedAt;
    });

    const int effectiveLimit = limit > 0 ? limit : 100;

    if (static_cast<int>(all.size()) > effectiveLimit)
    {
        result.truncated = true;
        all.resize(static_cast<std::size_t>(effectiveLimit));
    }

    result.workspaces = std::move(all);

    return foundation::Result<WorkspaceListResult>(std::move(result));
}

foundation::Result<WorkspaceMetadata> WorkspaceStore::getMetadata(const std::string& workspaceId) const
{
    if (!isValidWorkspaceId(workspaceId))
    {
        return foundation::Result<WorkspaceMetadata>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Workspace not found."));
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    const foundation::Path workspaceDir = workspaceDirectory(workspaceId);

    if (!std::filesystem::is_directory(workspaceDir.string()))
    {
        return foundation::Result<WorkspaceMetadata>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Workspace not found."));
    }

    return loadMetadata(workspaceDir);
}

foundation::Result<WorkspaceMetadata> WorkspaceStore::updateMetadata(const std::string& workspaceId,
                                                                     const WorkspaceUpdateRequest& request)
{
    if (!isValidWorkspaceId(workspaceId))
    {
        return foundation::Result<WorkspaceMetadata>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Workspace not found."));
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    const foundation::Path workspaceDir = workspaceDirectory(workspaceId);
    auto metadataResult = loadMetadata(workspaceDir);

    if (!metadataResult)
    {
        return metadataResult;
    }

    WorkspaceMetadata metadata = std::move(*metadataResult);

    if (request.name.has_value())
    {
        if (request.name->empty() || request.name->size() > 256U)
        {
            return foundation::Result<WorkspaceMetadata>(foundation::Error(
                foundation::ErrorCode::InvalidArgument, "Workspace name is required (max 256 characters)."));
        }

        metadata.name = *request.name;
    }

    if (request.description.has_value())
    {
        metadata.description = *request.description;
    }

    if (request.sourceRef.has_value())
    {
        metadata.sourceRef = *request.sourceRef;
    }

    metadata.updatedAt = currentTimestampIso();

    const auto saveResult = saveMetadata(workspaceDir, metadata);

    if (!saveResult)
    {
        return foundation::Result<WorkspaceMetadata>(saveResult.error());
    }

    return foundation::Result<WorkspaceMetadata>(std::move(metadata));
}

foundation::Result<bool> WorkspaceStore::remove(const std::string& workspaceId)
{
    if (!isValidWorkspaceId(workspaceId))
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Workspace not found."));
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    const foundation::Path workspaceDir = workspaceDirectory(workspaceId);

    if (!std::filesystem::is_directory(workspaceDir.string()))
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Workspace not found."));
    }

    std::error_code errorCode;
    std::filesystem::remove_all(workspaceDir.string(), errorCode);

    if (errorCode)
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::IOError, "Failed to delete workspace."));
    }

    return foundation::Result<bool>(true);
}

foundation::Result<foundation::Path> WorkspaceStore::resolveSnapshotPath(const std::string& workspaceId) const
{
    if (!isValidWorkspaceId(workspaceId))
    {
        return foundation::Result<foundation::Path>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Workspace not found."));
    }

    const foundation::Path workspaceDir = workspaceDirectory(workspaceId);
    const auto metadataResult = loadMetadata(workspaceDir);

    if (!metadataResult)
    {
        return foundation::Result<foundation::Path>(metadataResult.error());
    }

    const foundation::Path snapshotPath =
        foundation::Path(workspaceDir.string() + "/" + metadataResult->snapshotFile);
    const auto guardResult = ensureUnderRoot(snapshotPath);

    if (!guardResult)
    {
        return foundation::Result<foundation::Path>(guardResult.error());
    }

    return foundation::Result<foundation::Path>(snapshotPath);
}

foundation::Result<foundation::Path> WorkspaceStore::snapshotPathFor(const std::string& workspaceId) const
{
    return resolveSnapshotPath(workspaceId);
}

void WorkspaceStore::touchUpdatedAt(const std::string& workspaceId)
{
    if (!isValidWorkspaceId(workspaceId))
    {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    const foundation::Path workspaceDir = workspaceDirectory(workspaceId);
    auto metadataResult = loadMetadata(workspaceDir);

    if (!metadataResult)
    {
        return;
    }

    metadataResult->updatedAt = currentTimestampIso();

    if (!saveMetadata(workspaceDir, *metadataResult))
    {
        return;
    }
}

void WorkspaceStore::updateSummaryFromService(const std::string& workspaceId,
                                              const application::ApplicationService& service)
{
    if (!isValidWorkspaceId(workspaceId))
    {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    const foundation::Path workspaceDir = workspaceDirectory(workspaceId);
    auto metadataResult = loadMetadata(workspaceDir);

    if (!metadataResult)
    {
        return;
    }

    metadataResult->summary.hasModel = service.hasModel();

    if (service.hasModel())
    {
        metadataResult->summary.lineCount = service.model().totalLines();
        metadataResult->summary.errorCount = service.model().levelCounts().errorLines();
    }

    metadataResult->updatedAt = currentTimestampIso();

    if (!saveMetadata(workspaceDir, *metadataResult))
    {
        return;
    }
}

} // namespace scope::web
