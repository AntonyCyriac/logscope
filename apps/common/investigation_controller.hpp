/**
 * @file investigation_controller.hpp
 * @brief Thin investigation orchestration for desktop (P2 / v2.11.0).
 */

#pragma once

#include <mutex>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

#include "artifact_record.hpp"
#include "correlation_suggestion.hpp"
#include "crash_report.hpp"
#include "evidence_link.hpp"
#include "foundation/path.hpp"
#include "foundation/result.hpp"
#include "timeline_event.hpp"
#include "workspace.hpp"

namespace scope::application
{

/**
 * @brief Delegates to workspace::Investigation — no desktop-specific domain types.
 */
class InvestigationController
{
  public:
    explicit InvestigationController(foundation::Path investigationsRootDirectory);

    [[nodiscard]] const foundation::Path& investigationsRootDirectory() const noexcept;

    [[nodiscard]] bool isOpen() const noexcept;

    [[nodiscard]] foundation::Path investigationDirectory() const;

    [[nodiscard]] const scope::workspace::InvestigationManifest& manifest() const;

    [[nodiscard]] foundation::Result<scope::workspace::InvestigationManifest> create(const std::string& name,
                                                                                     const std::string& description = {});

    [[nodiscard]] foundation::Result<scope::workspace::InvestigationManifest> open(const foundation::Path& investigationDir);

    void close();

    [[nodiscard]] foundation::Result<scope::workspace::ArtifactRecord> addLogArtifact(
        const foundation::Path& sourcePath, const std::string& displayName = {});

    [[nodiscard]] foundation::Result<scope::workspace::ArtifactRecord> addArtifactFile(
        const foundation::Path& sourcePath, const std::string& type, const std::string& displayName = {});

    [[nodiscard]] foundation::Result<bool> removeArtifact(const std::string& artifactId);

    [[nodiscard]] foundation::Result<scope::workspace::TimelineProjectionResult> projectTimeline(
        scope::workspace::TimelineProjectionOptions options = {}) const;

    [[nodiscard]] foundation::Result<scope::workspace::CrashReport> analyzeCrash(const std::string& artifactId) const;

    [[nodiscard]] foundation::Result<std::vector<scope::workspace::EvidenceLinkRecord>> listEvidenceLinks() const;

    [[nodiscard]] foundation::Result<scope::workspace::EvidenceLinkRecord> addEvidenceLink(
        const scope::workspace::EvidenceLinkCreateRequest& request);

    [[nodiscard]] foundation::Result<bool> removeEvidenceLink(const std::string& linkId);

    [[nodiscard]] foundation::Result<scope::workspace::CorrelationSuggestionListResult> listCorrelationSuggestions(
        const scope::workspace::CorrelationSuggestionQuery& query,
        const std::unordered_set<std::string>& dismissedSuggestionIds = {}) const;

    [[nodiscard]] foundation::Result<scope::workspace::EvidenceLinkRecord> acceptCorrelationSuggestion(
        const std::string& suggestionId, const std::unordered_set<std::string>& dismissedSuggestionIds,
        std::optional<scope::workspace::EvidenceLinkType> type = std::nullopt,
        std::optional<std::string> note = std::nullopt);

    [[nodiscard]] foundation::Result<foundation::Path> resolveLogArtifactPath(const std::string& artifactId) const;

    [[nodiscard]] foundation::Result<foundation::Path> resolveArtifactDataPath(const std::string& artifactId) const;

    [[nodiscard]] foundation::Result<std::string> readArtifactText(const std::string& artifactId) const;

    [[nodiscard]] static std::string inferArtifactType(const std::string& explicitType, const foundation::Path& sourceFile);

  private:
    [[nodiscard]] foundation::Result<scope::workspace::Investigation> openInvestigation() const;

    [[nodiscard]] foundation::Path investigationDirectoryForId(const std::string& investigationId) const;

    foundation::Path m_rootDirectory;
    std::optional<scope::workspace::Investigation> m_investigation;
    mutable std::mutex m_mutex;
};

} // namespace scope::application
