/**
 * @file investigation_container.cpp
 * @brief Investigation aggregate root implementation.
 */

#include "investigation_container.hpp"

#include "artifact_handler.hpp"
#include "crash_analyzer.hpp"
#include "investigation_manifest_io.hpp"
#include "timeline_projector.hpp"

#include "foundation/clock.hpp"
#include "foundation/uuid.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <fstream>

namespace scope::workspace
{

namespace
{

constexpr const char* legacyWorkspaceFileName = "workspace.json";

std::string currentTimestampIso()
{
    return foundation::Clock::now().toString();
}

const ArtifactRecord* findArtifactById(const InvestigationManifest& manifest, const std::string& artifactId)
{
    const auto iterator = std::find_if(manifest.artifacts.begin(), manifest.artifacts.end(),
                                       [&artifactId](const ArtifactRecord& artifact) {
                                           return artifact.id == artifactId;
                                       });

    if (iterator == manifest.artifacts.end())
    {
        return nullptr;
    }

    return &(*iterator);
}

} // namespace

Investigation::Investigation(foundation::Path investigationDir, InvestigationManifest manifest)
    : m_rootDirectory(std::move(investigationDir))
    , m_manifest(std::move(manifest))
{
}

foundation::Result<Investigation> Investigation::create(const foundation::Path& investigationDir,
                                                      InvestigationCreateRequest request)
{
    if (request.name.empty() || request.name.size() > 256U)
    {
        return foundation::Result<Investigation>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Investigation name is required (max 256 characters)."));
    }

    std::error_code errorCode;
    std::filesystem::create_directories(investigationDir.string(), errorCode);

    if (errorCode)
    {
        return foundation::Result<Investigation>(
            foundation::Error(foundation::ErrorCode::IOError, "Failed to create investigation directory."));
    }

    const std::string timestamp = currentTimestampIso();
    InvestigationManifest manifest;
    manifest.schemaVersion = 1;
    manifest.id = foundation::Uuid::generate().toString();
    manifest.name = std::move(request.name);
    manifest.description = std::move(request.description);
    manifest.createdAt = timestamp;
    manifest.updatedAt = timestamp;

    const auto saveResult = saveManifest(investigationDir, manifest);

    if (!saveResult)
    {
        return foundation::Result<Investigation>(saveResult.error());
    }

    return foundation::Result<Investigation>(Investigation(investigationDir, std::move(manifest)));
}

foundation::Result<Investigation> Investigation::open(const foundation::Path& investigationDir)
{
    if (!std::filesystem::is_directory(investigationDir.string()))
    {
        return foundation::Result<Investigation>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Investigation not found."));
    }

    auto manifestResult = loadManifest(investigationDir);

    if (!manifestResult)
    {
        const foundation::Path legacyPath =
            foundation::Path(investigationDir.string() + "/" + legacyWorkspaceFileName);
        std::ifstream legacyStream(legacyPath.string());

        if (!legacyStream)
        {
            return foundation::Result<Investigation>(manifestResult.error());
        }

        std::ostringstream buffer;
        buffer << legacyStream.rdbuf();

        manifestResult = migrateLegacyWorkspaceMetadata(investigationDir, buffer.str());

        if (!manifestResult)
        {
            return foundation::Result<Investigation>(manifestResult.error());
        }
    }

    return foundation::Result<Investigation>(Investigation(investigationDir, std::move(*manifestResult)));
}

const foundation::Path& Investigation::rootDirectory() const noexcept
{
    return m_rootDirectory;
}

const InvestigationManifest& Investigation::manifest() const noexcept
{
    return m_manifest;
}

foundation::Result<ArtifactRecord> Investigation::addArtifact(ArtifactIngestRequest request)
{
    const IArtifactHandler* handler = findArtifactHandler(request.type);

    if (handler == nullptr)
    {
        return foundation::Result<ArtifactRecord>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Unknown artifact type."));
    }

    const std::string artifactId = foundation::Uuid::generate().toString();
    const foundation::Path artifactDirectory =
        foundation::Path(m_rootDirectory.string() + "/artifacts/" + artifactId);

    std::error_code errorCode;
    std::filesystem::create_directories(artifactDirectory.string(), errorCode);

    if (errorCode)
    {
        return foundation::Result<ArtifactRecord>(
            foundation::Error(foundation::ErrorCode::IOError, "Failed to create artifact directory."));
    }

    ArtifactIngestContext context;
    context.investigationRoot = m_rootDirectory;
    context.artifactDirectory = artifactDirectory;
    context.artifactId = artifactId;

    auto ingestResult = handler->ingest(context, request);

    if (!ingestResult)
    {
        std::filesystem::remove_all(artifactDirectory.string(), errorCode);

        return foundation::Result<ArtifactRecord>(ingestResult.error());
    }

    ArtifactRecord record = std::move(*ingestResult);

    if (!request.role.empty())
    {
        record.metadata["role"] = request.role;
    }

    m_manifest.artifacts.push_back(record);

    if (record.type == "log" && m_manifest.primaryArtifactId.empty())
    {
        m_manifest.primaryArtifactId = record.id;
    }

    m_manifest.updatedAt = currentTimestampIso();

    return foundation::Result<ArtifactRecord>(m_manifest.artifacts.back());
}

foundation::Result<bool> Investigation::removeArtifact(const std::string& artifactId)
{
    const auto iterator = std::find_if(m_manifest.artifacts.begin(), m_manifest.artifacts.end(),
                                       [&artifactId](const ArtifactRecord& artifact) {
                                           return artifact.id == artifactId;
                                       });

    if (iterator == m_manifest.artifacts.end())
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Artifact not found."));
    }

    const foundation::Path artifactDirectory =
        foundation::Path(m_rootDirectory.string() + "/artifacts/" + artifactId);

    std::error_code errorCode;
    std::filesystem::remove_all(artifactDirectory.string(), errorCode);

    if (errorCode)
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::IOError, "Failed to remove artifact directory."));
    }

    const bool removedPrimary = m_manifest.primaryArtifactId == artifactId;
    m_manifest.artifacts.erase(iterator);

    if (removedPrimary)
    {
        m_manifest.primaryArtifactId =
            m_manifest.artifacts.empty() ? std::string() : m_manifest.artifacts.front().id;
    }

    m_manifest.updatedAt = currentTimestampIso();

    return foundation::Result<bool>(true);
}

foundation::Result<ArtifactRecord> Investigation::entryArtifact() const
{
    if (m_manifest.primaryArtifactId.empty())
    {
        return foundation::Result<ArtifactRecord>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Entry artifact not set."));
    }

    const ArtifactRecord* artifact = findArtifactById(m_manifest, m_manifest.primaryArtifactId);

    if (artifact == nullptr)
    {
        return foundation::Result<ArtifactRecord>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Entry artifact not found."));
    }

    return foundation::Result<ArtifactRecord>(*artifact);
}

foundation::Result<ArtifactRecord> Investigation::artifactById(const std::string& artifactId) const
{
    const ArtifactRecord* artifact = findArtifactById(m_manifest, artifactId);

    if (artifact == nullptr)
    {
        return foundation::Result<ArtifactRecord>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Artifact not found."));
    }

    return foundation::Result<ArtifactRecord>(*artifact);
}

foundation::Result<foundation::Path> Investigation::entryArtifactDataPath() const
{
    const auto artifactResult = entryArtifact();

    if (!artifactResult)
    {
        return foundation::Result<foundation::Path>(artifactResult.error());
    }

    const IArtifactHandler* handler = findArtifactHandler(artifactResult->type);

    if (handler == nullptr)
    {
        return foundation::Result<foundation::Path>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Unknown artifact type."));
    }

    return handler->resolveDataPath(m_rootDirectory, *artifactResult);
}

foundation::Result<foundation::Path> Investigation::logArtifactDataPath(const std::string& artifactId) const
{
    const auto artifactResult = artifactById(artifactId);

    if (!artifactResult)
    {
        return foundation::Result<foundation::Path>(artifactResult.error());
    }

    if (artifactResult->type != "log")
    {
        return foundation::Result<foundation::Path>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Artifact is not a log."));
    }

    const IArtifactHandler* handler = findArtifactHandler(artifactResult->type);

    if (handler == nullptr)
    {
        return foundation::Result<foundation::Path>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Unknown artifact type."));
    }

    return handler->resolveDataPath(m_rootDirectory, *artifactResult);
}

foundation::Result<foundation::Path> Investigation::snapshotPath() const
{
    const foundation::Path path =
        foundation::Path(m_rootDirectory.string() + "/" + m_manifest.snapshotFile);
    const auto guardResult = ensureArtifactUnderRoot(path);

    if (!guardResult)
    {
        return foundation::Result<foundation::Path>(guardResult.error());
    }

    return foundation::Result<foundation::Path>(path);
}

foundation::Result<bool> Investigation::setEntryArtifact(const std::string& artifactId)
{
    const ArtifactRecord* artifact = findArtifactById(m_manifest, artifactId);

    if (artifact == nullptr)
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Artifact not found."));
    }

    if (artifact->type != "log")
    {
        return foundation::Result<bool>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Entry artifact must be a log."));
    }

    m_manifest.primaryArtifactId = artifactId;
    m_manifest.updatedAt = currentTimestampIso();

    return foundation::Result<bool>(true);
}

foundation::Result<bool> Investigation::touchUpdatedAt()
{
    m_manifest.updatedAt = currentTimestampIso();

    return foundation::Result<bool>(true);
}

foundation::Result<bool> Investigation::updateSummary(const InvestigationSummary& summary)
{
    m_manifest.summary = summary;
    m_manifest.updatedAt = currentTimestampIso();

    return foundation::Result<bool>(true);
}

foundation::Result<bool> Investigation::persist()
{
    return saveManifest(m_rootDirectory, m_manifest);
}

foundation::Result<TimelineProjectionResult> Investigation::projectTimeline(TimelineProjectionOptions options) const
{
    return TimelineProjector::project(m_rootDirectory, m_manifest, options);
}

foundation::Result<CrashReport> Investigation::analyzeCrash(const std::string& artifactId) const
{
    const auto artifactResult = artifactById(artifactId);

    if (!artifactResult)
    {
        return foundation::Result<CrashReport>(artifactResult.error());
    }

    const IArtifactCrashAnalyzer* analyzer = findCrashAnalyzer(artifactResult->type);

    if (analyzer == nullptr || !analyzer->supports(*artifactResult))
    {
        return foundation::Result<CrashReport>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "ARTIFACT_NOT_ANALYZABLE"));
    }

    const IArtifactHandler* handler = findArtifactHandler(artifactResult->type);

    if (handler == nullptr)
    {
        return foundation::Result<CrashReport>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Unknown artifact type."));
    }

    const auto dataPathResult = handler->resolveDataPath(m_rootDirectory, *artifactResult);

    if (!dataPathResult)
    {
        return foundation::Result<CrashReport>(dataPathResult.error());
    }

    if (!analyzer->canAnalyze(*artifactResult, *dataPathResult))
    {
        return foundation::Result<CrashReport>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Artifact data is not readable."));
    }

    CrashAnalysisContext context;
    context.investigationId = m_manifest.id;
    context.investigationRoot = m_rootDirectory;

    return analyzer->analyze(*artifactResult, *dataPathResult, context);
}

foundation::Result<bool> Investigation::ensureArtifactUnderRoot(const foundation::Path& path) const
{
    std::error_code errorCode;
    const std::filesystem::path absolutePath = std::filesystem::weakly_canonical(path.string(), errorCode);
    const std::filesystem::path absoluteRoot =
        std::filesystem::weakly_canonical(m_rootDirectory.string(), errorCode);

    if (errorCode)
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Invalid investigation path."));
    }

    const std::string absoluteString = absolutePath.string();
    const std::string rootString = absoluteRoot.string();

    if (absoluteString.rfind(rootString, 0) != 0)
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Path escapes investigation directory."));
    }

    return foundation::Result<bool>(true);
}

} // namespace scope::workspace
