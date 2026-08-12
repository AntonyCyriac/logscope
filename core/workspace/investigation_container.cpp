/**
 * @file investigation_container.cpp
 * @brief Investigation aggregate root implementation.
 */

#include "investigation_container.hpp"

#include "artifact_handler.hpp"
#include "crash_analyzer.hpp"
#include "evidence_link.hpp"
#include "investigation_manifest_io.hpp"
#include "correlation_engine.hpp"
#include "timeline_projector.hpp"

#include "foundation/clock.hpp"
#include "foundation/uuid.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_set>

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

std::unordered_set<std::string> collectTimelineEventIds(const Investigation& investigation)
{
    TimelineProjectionOptions options;
    options.limit = 10'000U;
    options.offset = 0U;
    options.order = TimelineSortOrder::Ascending;

    const auto timelineResult = investigation.projectTimeline(options);

    std::unordered_set<std::string> eventIds;

    if (!timelineResult)
    {
        return eventIds;
    }

    for (const TimelineEvent& event : timelineResult->events)
    {
        eventIds.insert(event.id);
    }

    return eventIds;
}

EvidenceLinkRecord annotateLinkStatus(const EvidenceLink& link, const std::unordered_set<std::string>& eventIds)
{
    EvidenceLinkRecord record;
    static_cast<EvidenceLink&>(record) = link;

    const bool sourceActive = eventIds.count(link.source.eventId) > 0U;
    const bool targetActive = eventIds.count(link.target.eventId) > 0U;
    record.status = (sourceActive && targetActive) ? EvidenceLinkStatus::Active : EvidenceLinkStatus::Stale;

    return record;
}

constexpr std::size_t kMaxEvidenceLinkNoteLength = 2'000U;

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
    const CrashReportProvider provider = [this](const std::string& artifactId) {
        return analyzeCrash(artifactId);
    };

    return TimelineProjector::project(m_rootDirectory, m_manifest, options, &provider);
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
        CrashReport report;
        report.id = makeCrashReportId(m_manifest.id, artifactResult->id, "unsupported");
        report.artifactId = artifactResult->id;
        report.artifactType = artifactResult->type;
        report.status = CrashAnalysisStatus::NotSupported;
        report.summary = "Artifact type does not support crash analysis";

        return foundation::Result<CrashReport>(std::move(report));
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
        CrashReport report;
        report.id = makeCrashReportId(m_manifest.id, artifactResult->id, std::string(analyzer->artifactType()) + "-unreadable");
        report.artifactId = artifactResult->id;
        report.artifactType = artifactResult->type;
        report.status = CrashAnalysisStatus::Failed;
        report.summary = "Artifact data is not readable";
        report.warnings.push_back("Artifact data file is missing or not readable.");

        return foundation::Result<CrashReport>(std::move(report));
    }

    CrashAnalysisContext context;
    context.investigationId = m_manifest.id;
    context.investigationRoot = m_rootDirectory;

    return analyzer->analyze(*artifactResult, *dataPathResult, context);
}

foundation::Result<std::vector<EvidenceLinkRecord>> Investigation::listEvidenceLinks() const
{
    const std::unordered_set<std::string> eventIds = collectTimelineEventIds(*this);
    std::vector<EvidenceLinkRecord> records;
    records.reserve(m_manifest.evidenceLinks.size());

    for (const EvidenceLink& link : m_manifest.evidenceLinks)
    {
        records.push_back(annotateLinkStatus(link, eventIds));
    }

    return foundation::Result<std::vector<EvidenceLinkRecord>>(std::move(records));
}

foundation::Result<EvidenceLinkRecord> Investigation::addEvidenceLink(EvidenceLinkCreateRequest request)
{
    if (request.source.kind != "timeline_event" || request.target.kind != "timeline_event")
    {
        return foundation::Result<EvidenceLinkRecord>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Link endpoints must use kind timeline_event."));
    }

    if (request.source.eventId.empty() || request.target.eventId.empty())
    {
        return foundation::Result<EvidenceLinkRecord>(foundation::Error(
            foundation::ErrorCode::InvalidLinkTarget, "Timeline event id is required."));
    }

    if (request.source.eventId == request.target.eventId)
    {
        return foundation::Result<EvidenceLinkRecord>(foundation::Error(
            foundation::ErrorCode::InvalidLinkTarget, "source and target must differ"));
    }

    if (request.note.has_value() && request.note->size() > kMaxEvidenceLinkNoteLength)
    {
        return foundation::Result<EvidenceLinkRecord>(foundation::Error(
            foundation::ErrorCode::InvalidArgument,
            "Link note exceeds maximum length of 2000 characters."));
    }

    const std::unordered_set<std::string> eventIds = collectTimelineEventIds(*this);

    if (eventIds.count(request.source.eventId) == 0U)
    {
        return foundation::Result<EvidenceLinkRecord>(foundation::Error(
            foundation::ErrorCode::InvalidLinkTarget,
            "Timeline event not found: " + request.source.eventId));
    }

    if (eventIds.count(request.target.eventId) == 0U)
    {
        return foundation::Result<EvidenceLinkRecord>(foundation::Error(
            foundation::ErrorCode::InvalidLinkTarget,
            "Timeline event not found: " + request.target.eventId));
    }

    for (const EvidenceLink& existing : m_manifest.evidenceLinks)
    {
        if (existing.source.eventId == request.source.eventId && existing.target.eventId == request.target.eventId
            && existing.type == request.type)
        {
            return foundation::Result<EvidenceLinkRecord>(foundation::Error(
                foundation::ErrorCode::DuplicateEvidenceLink, "Evidence link already exists."));
        }
    }

    EvidenceLink link;
    link.id = foundation::Uuid::generate().toString();
    link.type = request.type;
    link.source = request.source;
    link.target = request.target;
    link.createdAt = currentTimestampIso();

    if (request.note.has_value() && !request.note->empty())
    {
        link.note = *request.note;
    }

    m_manifest.evidenceLinks.push_back(link);
    m_manifest.schemaVersion = 2;
    m_manifest.updatedAt = currentTimestampIso();

    const auto persistResult = persist();

    if (!persistResult)
    {
        m_manifest.evidenceLinks.pop_back();

        return foundation::Result<EvidenceLinkRecord>(persistResult.error());
    }

    return foundation::Result<EvidenceLinkRecord>(annotateLinkStatus(link, eventIds));
}

foundation::Result<CorrelationSuggestionListResult> Investigation::listCorrelationSuggestions(
    CorrelationSuggestionQuery query, const std::unordered_set<std::string>& dismissedSuggestionIds) const
{
    TimelineProjectionOptions options;
    options.limit = 10'000U;
    options.offset = 0U;
    options.order = TimelineSortOrder::Ascending;

    const auto timelineResult = projectTimeline(options);

    if (!timelineResult)
    {
        return foundation::Result<CorrelationSuggestionListResult>(timelineResult.error());
    }

    return foundation::Result<CorrelationSuggestionListResult>(CorrelationEngine::computeSuggestions(
        m_manifest.id, timelineResult->events, m_manifest.evidenceLinks, dismissedSuggestionIds, std::move(query)));
}

foundation::Result<EvidenceLinkRecord> Investigation::acceptCorrelationSuggestion(
    const std::string& suggestionId, const std::optional<EvidenceLinkType> type,
    const std::optional<std::string> note, const std::unordered_set<std::string>& dismissedSuggestionIds)
{
    std::optional<CorrelationSuggestion> matchedSuggestion;

    for (int offset = 0;; offset += 50)
    {
        CorrelationSuggestionQuery query;
        query.limit = 50;
        query.offset = offset;

        const auto suggestionsResult = listCorrelationSuggestions(query, dismissedSuggestionIds);

        if (!suggestionsResult)
        {
            return foundation::Result<EvidenceLinkRecord>(suggestionsResult.error());
        }

        for (const CorrelationSuggestion& suggestion : suggestionsResult->suggestions)
        {
            if (suggestion.id == suggestionId)
            {
                matchedSuggestion = suggestion;
                break;
            }
        }

        if (matchedSuggestion.has_value() || offset + 50 >= suggestionsResult->total)
        {
            break;
        }
    }

    if (!matchedSuggestion.has_value())
    {
        return foundation::Result<EvidenceLinkRecord>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Correlation suggestion not found."));
    }

    const std::unordered_set<std::string> eventIds = collectTimelineEventIds(*this);

    if (eventIds.count(matchedSuggestion->sourceEventId) == 0U
        || eventIds.count(matchedSuggestion->targetEventId) == 0U)
    {
        return foundation::Result<EvidenceLinkRecord>(
            foundation::Error(foundation::ErrorCode::StaleSuggestion,
                              "Timeline events for suggestion are no longer available."));
    }

    EvidenceLinkCreateRequest createRequest;
    createRequest.type = type.value_or(EvidenceLinkType::Related);
    createRequest.source.kind = "timeline_event";
    createRequest.source.eventId = matchedSuggestion->sourceEventId;
    createRequest.target.kind = "timeline_event";
    createRequest.target.eventId = matchedSuggestion->targetEventId;

    if (note.has_value() && !note->empty())
    {
        createRequest.note = *note;
    }
    else
    {
        createRequest.note = matchedSuggestion->summary;
    }

    return addEvidenceLink(createRequest);
}

foundation::Result<bool> Investigation::removeEvidenceLink(const std::string& linkId)
{
    const auto iterator = std::find_if(m_manifest.evidenceLinks.begin(), m_manifest.evidenceLinks.end(),
                                       [&linkId](const EvidenceLink& link) { return link.id == linkId; });

    if (iterator == m_manifest.evidenceLinks.end())
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Evidence link not found."));
    }

    m_manifest.evidenceLinks.erase(iterator);
    m_manifest.updatedAt = currentTimestampIso();

    return persist();
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
