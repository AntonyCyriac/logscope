/**
 * @file workspace_store.cpp
 */

#include "workspace_store.hpp"

#include "investigation_store.hpp"

#include <algorithm>

namespace scope::web
{

namespace
{

WorkspaceSourceRef sourceRefFromManifest(const scope::workspace::InvestigationManifest& manifest)
{
    WorkspaceSourceRef ref;

    if (manifest.primaryArtifactId.empty())
    {
        return ref;
    }

    const auto iterator = std::find_if(manifest.artifacts.begin(), manifest.artifacts.end(),
                                       [&manifest](const scope::workspace::ArtifactRecord& artifact) {
                                           return artifact.id == manifest.primaryArtifactId;
                                       });

    if (iterator == manifest.artifacts.end())
    {
        return ref;
    }

    ref.type = iterator->source.kind.empty() ? iterator->type : iterator->source.kind;
    ref.displayName = iterator->source.displayName.empty() ? iterator->name : iterator->source.displayName;
    ref.path = iterator->relativePath;

    return ref;
}

WorkspaceSummary summaryFromManifest(const scope::workspace::InvestigationManifest& manifest)
{
    WorkspaceSummary summary;
    summary.hasModel = manifest.summary.hasModel;
    summary.lineCount = manifest.summary.lineCount;
    summary.errorCount = manifest.summary.errorCount;

    return summary;
}

WorkspaceMetadata metadataFromManifest(const scope::workspace::InvestigationManifest& manifest)
{
    WorkspaceMetadata metadata;
    metadata.id = manifest.id;
    metadata.name = manifest.name;
    metadata.description = manifest.description;
    metadata.createdAt = manifest.createdAt;
    metadata.updatedAt = manifest.updatedAt;
    metadata.sourceRef = sourceRefFromManifest(manifest);
    metadata.summary = summaryFromManifest(manifest);
    metadata.snapshotFile = manifest.snapshotFile;

    if (metadata.snapshotFile.empty())
    {
        metadata.snapshotFile = "snapshot.session";
    }

    return metadata;
}

} // namespace

WorkspaceStore::WorkspaceStore(const WebConfig& config)
    : m_investigationStore(config)
{
}

const foundation::Path& WorkspaceStore::rootDirectory() const noexcept
{
    return m_investigationStore.rootDirectory();
}

bool WorkspaceStore::isValidWorkspaceId(const std::string& workspaceId)
{
    return InvestigationStore::isValidInvestigationId(workspaceId);
}

foundation::Result<WorkspaceMetadata> WorkspaceStore::create(const WorkspaceCreateRequest& request)
{
    const auto createResult = m_investigationStore.create(request.name, request.description);

    if (!createResult)
    {
        return foundation::Result<WorkspaceMetadata>(createResult.error());
    }

    if (request.sourceRef.has_value() && !request.sourceRef->path.empty())
    {
        const auto artifactResult = m_investigationStore.addLogArtifact(
            createResult->id, foundation::Path(request.sourceRef->path),
            request.sourceRef->displayName.empty() ? request.sourceRef->path : request.sourceRef->displayName);

        if (!artifactResult)
        {
            m_investigationStore.remove(createResult->id);

            return foundation::Result<WorkspaceMetadata>(artifactResult.error());
        }
    }

    const auto refreshed = m_investigationStore.get(createResult->id);

    if (!refreshed)
    {
        return foundation::Result<WorkspaceMetadata>(refreshed.error());
    }

    return foundation::Result<WorkspaceMetadata>(metadataFromManifest(*refreshed));
}

foundation::Result<WorkspaceListResult> WorkspaceStore::list(const int limit) const
{
    const auto listResult = m_investigationStore.list(limit);

    if (!listResult)
    {
        return foundation::Result<WorkspaceListResult>(listResult.error());
    }

    WorkspaceListResult result;
    result.truncated = listResult->truncated;

    for (const scope::workspace::InvestigationManifest& manifest : listResult->investigations)
    {
        result.workspaces.push_back(metadataFromManifest(manifest));
    }

    return foundation::Result<WorkspaceListResult>(std::move(result));
}

foundation::Result<WorkspaceMetadata> WorkspaceStore::getMetadata(const std::string& workspaceId) const
{
    const auto manifestResult = m_investigationStore.get(workspaceId);

    if (!manifestResult)
    {
        return foundation::Result<WorkspaceMetadata>(manifestResult.error());
    }

    return foundation::Result<WorkspaceMetadata>(metadataFromManifest(*manifestResult));
}

foundation::Result<WorkspaceMetadata> WorkspaceStore::updateMetadata(const std::string& workspaceId,
                                                                     const WorkspaceUpdateRequest& request)
{
    InvestigationUpdateRequest updateRequest;

    if (request.name.has_value())
    {
        updateRequest.name = request.name;
    }

    if (request.description.has_value())
    {
        updateRequest.description = request.description;
    }

    const auto updateResult = m_investigationStore.update(workspaceId, updateRequest);

    if (!updateResult)
    {
        return foundation::Result<WorkspaceMetadata>(updateResult.error());
    }

    return foundation::Result<WorkspaceMetadata>(metadataFromManifest(*updateResult));
}

foundation::Result<bool> WorkspaceStore::remove(const std::string& workspaceId)
{
    return m_investigationStore.remove(workspaceId);
}

foundation::Result<foundation::Path> WorkspaceStore::resolveSnapshotPath(const std::string& workspaceId) const
{
    return m_investigationStore.resolveSnapshotPath(workspaceId);
}

foundation::Result<foundation::Path> WorkspaceStore::snapshotPathFor(const std::string& workspaceId) const
{
    return m_investigationStore.snapshotPathFor(workspaceId);
}

void WorkspaceStore::touchUpdatedAt(const std::string& workspaceId)
{
    m_investigationStore.touchUpdatedAt(workspaceId);
}

void WorkspaceStore::updateSummaryFromService(const std::string& workspaceId,
                                              const application::ApplicationService& service)
{
    m_investigationStore.updateSummaryFromService(workspaceId, service);
}

InvestigationStore& WorkspaceStore::investigationStore() noexcept
{
    return m_investigationStore;
}

const InvestigationStore& WorkspaceStore::investigationStore() const noexcept
{
    return m_investigationStore;
}

} // namespace scope::web
