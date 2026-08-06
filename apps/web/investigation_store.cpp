/**
 * @file investigation_store.cpp
 */

#include "investigation_store.hpp"

#include "investigation_manifest_io.hpp"
#include "timeline_event.hpp"
#include "timeline_projector.hpp"

#include "foundation/uuid.hpp"

#include <algorithm>
#include <filesystem>
#include <regex>

namespace scope::web
{

namespace
{

using scope::workspace::ArtifactIngestRequest;
using scope::workspace::ArtifactSource;
using scope::workspace::Investigation;
using scope::workspace::InvestigationCreateRequest;
using scope::workspace::InvestigationSummary;
using scope::workspace::loadManifest;
using scope::workspace::saveManifest;

} // namespace

InvestigationStore::InvestigationStore(const WebConfig& config)
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

const foundation::Path& InvestigationStore::rootDirectory() const noexcept
{
    return m_rootDirectory;
}

bool InvestigationStore::isValidInvestigationId(const std::string& investigationId)
{
    static const std::regex pattern("^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$");

    if (!std::regex_match(investigationId, pattern))
    {
        return false;
    }

    return static_cast<bool>(foundation::Uuid::parse(investigationId));
}

foundation::Path InvestigationStore::investigationDirectory(const std::string& investigationId) const
{
    return foundation::Path(m_rootDirectory.string() + "/" + investigationId);
}

foundation::Result<bool> InvestigationStore::ensureUnderRoot(const foundation::Path& path) const
{
    std::error_code errorCode;
    const std::filesystem::path absolutePath = std::filesystem::weakly_canonical(path.string(), errorCode);
    const std::filesystem::path absoluteRoot = std::filesystem::weakly_canonical(m_rootDirectory.string(), errorCode);

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

foundation::Result<Investigation> InvestigationStore::openInvestigation(const std::string& investigationId) const
{
    if (!isValidInvestigationId(investigationId))
    {
        return foundation::Result<Investigation>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Investigation not found."));
    }

    const foundation::Path investigationDir = investigationDirectory(investigationId);

    if (!std::filesystem::is_directory(investigationDir.string()))
    {
        return foundation::Result<Investigation>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Investigation not found."));
    }

    return Investigation::open(investigationDir);
}

foundation::Result<scope::workspace::InvestigationManifest> InvestigationStore::create(const std::string& name,
                                                                                        const std::string& description)
{
    if (name.empty() || name.size() > 256U)
    {
        return foundation::Result<scope::workspace::InvestigationManifest>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Investigation name is required (max 256 characters)."));
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    const foundation::Path stagingDir =
        foundation::Path(m_rootDirectory.string() + "/.staging-" + foundation::Uuid::generate().toString());

    InvestigationCreateRequest createRequest;
    createRequest.name = name;
    createRequest.description = description;

    const auto createResult = Investigation::create(stagingDir, createRequest);

    if (!createResult)
    {
        return foundation::Result<scope::workspace::InvestigationManifest>(createResult.error());
    }

    const std::string investigationId = createResult->manifest().id;
    const foundation::Path investigationDir = investigationDirectory(investigationId);

    std::error_code errorCode;
    std::filesystem::rename(stagingDir.string(), investigationDir.string(), errorCode);

    if (errorCode)
    {
        std::filesystem::remove_all(stagingDir.string(), errorCode);

        return foundation::Result<scope::workspace::InvestigationManifest>(
            foundation::Error(foundation::ErrorCode::IOError, "Failed to finalize investigation directory."));
    }

    return foundation::Result<scope::workspace::InvestigationManifest>(createResult->manifest());
}

foundation::Result<InvestigationListResult> InvestigationStore::list(const int limit) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    InvestigationListResult result;
    std::vector<scope::workspace::InvestigationManifest> all;

    std::error_code errorCode;

    if (!std::filesystem::is_directory(m_rootDirectory.string(), errorCode))
    {
        return foundation::Result<InvestigationListResult>(std::move(result));
    }

    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator(m_rootDirectory.string(), errorCode))
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

        const auto investigationResult = Investigation::open(foundation::Path(entry.path().string()));

        if (!investigationResult)
        {
            continue;
        }

        all.push_back(investigationResult->manifest());
    }

    std::sort(all.begin(), all.end(), [](const scope::workspace::InvestigationManifest& left,
                                         const scope::workspace::InvestigationManifest& right) {
        return left.updatedAt > right.updatedAt;
    });

    const int effectiveLimit = limit > 0 ? limit : 100;

    if (static_cast<int>(all.size()) > effectiveLimit)
    {
        result.truncated = true;
        all.resize(static_cast<std::size_t>(effectiveLimit));
    }

    result.investigations = std::move(all);

    return foundation::Result<InvestigationListResult>(std::move(result));
}

foundation::Result<scope::workspace::InvestigationManifest> InvestigationStore::get(
    const std::string& investigationId) const
{
    if (!isValidInvestigationId(investigationId))
    {
        return foundation::Result<scope::workspace::InvestigationManifest>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Investigation not found."));
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    const foundation::Path investigationDir = investigationDirectory(investigationId);

    if (!std::filesystem::is_directory(investigationDir.string()))
    {
        return foundation::Result<scope::workspace::InvestigationManifest>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Investigation not found."));
    }

    return loadManifest(investigationDir);
}

foundation::Result<scope::workspace::InvestigationManifest> InvestigationStore::update(
    const std::string& investigationId, const InvestigationUpdateRequest& request)
{
    if (request.name.has_value() && (request.name->empty() || request.name->size() > 256U))
    {
        return foundation::Result<scope::workspace::InvestigationManifest>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Investigation name is required (max 256 characters)."));
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    auto investigationResult = openInvestigation(investigationId);

    if (!investigationResult)
    {
        return foundation::Result<scope::workspace::InvestigationManifest>(investigationResult.error());
    }

    Investigation investigation = std::move(*investigationResult);

    if (request.primaryArtifactId.has_value())
    {
        const auto setEntryResult = investigation.setEntryArtifact(*request.primaryArtifactId);

        if (!setEntryResult)
        {
            return foundation::Result<scope::workspace::InvestigationManifest>(setEntryResult.error());
        }
    }

    scope::workspace::InvestigationManifest manifest = investigation.manifest();
    bool changed = request.primaryArtifactId.has_value();

    if (request.name.has_value())
    {
        manifest.name = *request.name;
        changed = true;
    }

    if (request.description.has_value())
    {
        manifest.description = *request.description;
        changed = true;
    }

    if (changed)
    {
        if (const auto touchResult = investigation.touchUpdatedAt(); !touchResult)
        {
            return foundation::Result<scope::workspace::InvestigationManifest>(touchResult.error());
        }

        manifest.updatedAt = investigation.manifest().updatedAt;

        const auto saveResult = saveManifest(investigation.rootDirectory(), manifest);

        if (!saveResult)
        {
            return foundation::Result<scope::workspace::InvestigationManifest>(saveResult.error());
        }
    }

    return foundation::Result<scope::workspace::InvestigationManifest>(manifest);
}

foundation::Result<bool> InvestigationStore::remove(const std::string& investigationId)
{
    if (!isValidInvestigationId(investigationId))
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Investigation not found."));
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    const foundation::Path investigationDir = investigationDirectory(investigationId);

    if (!std::filesystem::is_directory(investigationDir.string()))
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Investigation not found."));
    }

    std::error_code errorCode;
    std::filesystem::remove_all(investigationDir.string(), errorCode);

    if (errorCode)
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::IOError, "Failed to delete investigation."));
    }

    return foundation::Result<bool>(true);
}

foundation::Result<scope::workspace::ArtifactRecord> InvestigationStore::addLogArtifact(
    const std::string& investigationId, const foundation::Path& sourcePath, const std::string& displayName)
{
    return addArtifactFile(investigationId, sourcePath, displayName, "log");
}

foundation::Result<scope::workspace::ArtifactRecord> InvestigationStore::addArtifactFile(
    const std::string& investigationId, const foundation::Path& sourcePath, const std::string& displayName,
    const std::string& type, const std::string& role)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto investigationResult = openInvestigation(investigationId);

    if (!investigationResult)
    {
        return foundation::Result<scope::workspace::ArtifactRecord>(investigationResult.error());
    }

    Investigation investigation = std::move(*investigationResult);

    ArtifactIngestRequest request;
    request.type = type;
    request.name = displayName.empty() ? sourcePath.string() : displayName;
    request.sourceFile = sourcePath;
    request.source = ArtifactSource{"upload", request.name};
    request.role = role;

    const auto artifactResult = investigation.addArtifact(request);

    if (!artifactResult)
    {
        return foundation::Result<scope::workspace::ArtifactRecord>(artifactResult.error());
    }

    const auto persistResult = investigation.persist();

    if (!persistResult)
    {
        return foundation::Result<scope::workspace::ArtifactRecord>(persistResult.error());
    }

    return foundation::Result<scope::workspace::ArtifactRecord>(*artifactResult);
}

foundation::Result<scope::workspace::ArtifactRecord> InvestigationStore::addNoteArtifact(
    const std::string& investigationId, const std::string& title, const std::string& body)
{
    if (title.empty())
    {
        return foundation::Result<scope::workspace::ArtifactRecord>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Note title is required."));
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    auto investigationResult = openInvestigation(investigationId);

    if (!investigationResult)
    {
        return foundation::Result<scope::workspace::ArtifactRecord>(investigationResult.error());
    }

    Investigation investigation = std::move(*investigationResult);

    ArtifactIngestRequest request;
    request.type = "note";
    request.name = title;
    request.noteBody = body;
    request.source = ArtifactSource{"inline", title};

    const auto artifactResult = investigation.addArtifact(request);

    if (!artifactResult)
    {
        return foundation::Result<scope::workspace::ArtifactRecord>(artifactResult.error());
    }

    const auto persistResult = investigation.persist();

    if (!persistResult)
    {
        return foundation::Result<scope::workspace::ArtifactRecord>(persistResult.error());
    }

    return foundation::Result<scope::workspace::ArtifactRecord>(*artifactResult);
}

foundation::Result<bool> InvestigationStore::removeArtifact(const std::string& investigationId,
                                                            const std::string& artifactId)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto investigationResult = openInvestigation(investigationId);

    if (!investigationResult)
    {
        return foundation::Result<bool>(investigationResult.error());
    }

    Investigation investigation = std::move(*investigationResult);
    const auto removeResult = investigation.removeArtifact(artifactId);

    if (!removeResult)
    {
        return removeResult;
    }

    return investigation.persist();
}

foundation::Result<foundation::Path> InvestigationStore::resolveSnapshotPath(const std::string& investigationId) const
{
    const auto investigationResult = openInvestigation(investigationId);

    if (!investigationResult)
    {
        return foundation::Result<foundation::Path>(investigationResult.error());
    }

    return investigationResult->snapshotPath();
}

foundation::Result<foundation::Path> InvestigationStore::snapshotPathFor(const std::string& investigationId) const
{
    return resolveSnapshotPath(investigationId);
}

foundation::Result<foundation::Path> InvestigationStore::resolveLogArtifactPath(
    const std::string& investigationId, const std::string& artifactId) const
{
    const auto investigationResult = openInvestigation(investigationId);

    if (!investigationResult)
    {
        return foundation::Result<foundation::Path>(investigationResult.error());
    }

    if (artifactId.empty())
    {
        return investigationResult->entryArtifactDataPath();
    }

    return investigationResult->logArtifactDataPath(artifactId);
}

foundation::Result<foundation::Path> InvestigationStore::resolveEntryLogPath(const std::string& investigationId) const
{
    return resolveLogArtifactPath(investigationId, std::string());
}

void InvestigationStore::touchUpdatedAt(const std::string& investigationId)
{
    if (!isValidInvestigationId(investigationId))
    {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    auto investigationResult = openInvestigation(investigationId);

    if (!investigationResult)
    {
        return;
    }

    Investigation investigation = std::move(*investigationResult);

    if (!investigation.touchUpdatedAt())
    {
        return;
    }

    if (!investigation.persist())
    {
        return;
    }
}

foundation::Result<scope::workspace::TimelineProjectionResult> InvestigationStore::projectTimeline(
    const std::string& investigationId, scope::workspace::TimelineProjectionOptions options) const
{
    if (!isValidInvestigationId(investigationId))
    {
        return foundation::Result<scope::workspace::TimelineProjectionResult>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Investigation not found."));
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    const auto investigationResult = openInvestigation(investigationId);

    if (!investigationResult)
    {
        return foundation::Result<scope::workspace::TimelineProjectionResult>(investigationResult.error());
    }

    return investigationResult->projectTimeline(options);
}

foundation::Result<scope::workspace::CrashReport> InvestigationStore::analyzeCrash(
    const std::string& investigationId, const std::string& artifactId) const
{
    if (!isValidInvestigationId(investigationId))
    {
        return foundation::Result<scope::workspace::CrashReport>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Investigation not found."));
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    const auto investigationResult = openInvestigation(investigationId);

    if (!investigationResult)
    {
        return foundation::Result<scope::workspace::CrashReport>(investigationResult.error());
    }

    return investigationResult->analyzeCrash(artifactId);
}

foundation::Result<std::vector<scope::workspace::EvidenceLinkRecord>> InvestigationStore::listEvidenceLinks(
    const std::string& investigationId) const
{
    if (!isValidInvestigationId(investigationId))
    {
        return foundation::Result<std::vector<scope::workspace::EvidenceLinkRecord>>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Investigation not found."));
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    const auto investigationResult = openInvestigation(investigationId);

    if (!investigationResult)
    {
        return foundation::Result<std::vector<scope::workspace::EvidenceLinkRecord>>(investigationResult.error());
    }

    return investigationResult->listEvidenceLinks();
}

foundation::Result<scope::workspace::EvidenceLinkRecord> InvestigationStore::addEvidenceLink(
    const std::string& investigationId, scope::workspace::EvidenceLinkCreateRequest request)
{
    if (!isValidInvestigationId(investigationId))
    {
        return foundation::Result<scope::workspace::EvidenceLinkRecord>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Investigation not found."));
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    auto investigationResult = openInvestigation(investigationId);

    if (!investigationResult)
    {
        return foundation::Result<scope::workspace::EvidenceLinkRecord>(investigationResult.error());
    }

    Investigation investigation = std::move(*investigationResult);

    return investigation.addEvidenceLink(std::move(request));
}

foundation::Result<bool> InvestigationStore::removeEvidenceLink(const std::string& investigationId,
                                                                const std::string& linkId)
{
    if (!isValidInvestigationId(investigationId))
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Investigation not found."));
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    auto investigationResult = openInvestigation(investigationId);

    if (!investigationResult)
    {
        return foundation::Result<bool>(investigationResult.error());
    }

    Investigation investigation = std::move(*investigationResult);

    return investigation.removeEvidenceLink(linkId);
}

void InvestigationStore::updateSummaryFromService(const std::string& investigationId,
                                                  const application::ApplicationService& service)
{
    if (!isValidInvestigationId(investigationId))
    {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    auto investigationResult = openInvestigation(investigationId);

    if (!investigationResult)
    {
        return;
    }

    Investigation investigation = std::move(*investigationResult);

    InvestigationSummary summary;
    summary.hasModel = service.hasModel();

    if (service.hasModel())
    {
        summary.lineCount = service.model().totalLines();
        summary.errorCount = service.model().levelCounts().errorLines();
    }

    if (!investigation.updateSummary(summary))
    {
        return;
    }

    if (!investigation.persist())
    {
        return;
    }
}

} // namespace scope::web
