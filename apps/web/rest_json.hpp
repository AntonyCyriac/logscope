/**
 * @file rest_json.hpp
 * @brief REST JSON envelopes and domain mappers (M15.1).
 */

#pragma once

#include <string>
#include <vector>

#include "analytics_result.hpp"
#include "analysis_model.hpp"
#include "extension_info.hpp"
#include "foundation/error.hpp"
#include "foundation/path.hpp"
#include "application_service.hpp"
#include "analyze_job_queue.hpp"
#include "investigation_result.hpp"
#include "investigation_store.hpp"
#include "workspace_store.hpp"

namespace scope::web
{

[[nodiscard]] std::string escapeJsonString(std::string_view value);

[[nodiscard]] std::string successEnvelope(std::string_view dataJson);

[[nodiscard]] std::string errorEnvelope(std::string_view code, std::string_view message,
                                        std::string_view detailsJson = "{}");

[[nodiscard]] std::string errorEnvelopeFromFoundation(const foundation::Error& error);

[[nodiscard]] int httpStatusForError(const foundation::Error& error);

[[nodiscard]] std::string formatExtensionInfo(const extension::ExtensionInfo& info);

[[nodiscard]] std::string formatExtensionList(const std::vector<extension::ExtensionInfo>& extensions);

[[nodiscard]] std::string formatPathList(const std::vector<foundation::Path>& paths);

[[nodiscard]] std::string formatAnalyticsJson(const analytics::AnalyticsResult& result);

[[nodiscard]] std::string formatAnalyzeJson(const analysis::AnalysisModel& model);

[[nodiscard]] std::string formatInvestigationJson(const investigation::InvestigationResult& result);

[[nodiscard]] std::string formatAgentInvestigateJson(const application::AgentInvestigateResult& result);

[[nodiscard]] std::string formatWorkspaceMetadata(const WorkspaceMetadata& metadata);

[[nodiscard]] std::string formatWorkspaceList(const WorkspaceListResult& list);

[[nodiscard]] std::string formatWorkspaceOpenResult(const std::string& workspaceId, const foundation::Path& sourcePath,
                                                    const WorkspaceSummary& summary);

[[nodiscard]] std::string formatInvestigationManifest(const scope::workspace::InvestigationManifest& manifest);

[[nodiscard]] std::string formatInvestigationList(const InvestigationListResult& list);

[[nodiscard]] std::string formatArtifactRecord(const scope::workspace::ArtifactRecord& artifact,
                                               const std::string& primaryArtifactId = std::string());

[[nodiscard]] std::string formatInvestigationOpenResult(const std::string& investigationId,
                                                        const std::string& artifactId,
                                                        const std::string& artifactType,
                                                        const foundation::Path& sourcePath,
                                                        const scope::workspace::InvestigationSummary& summary,
                                                        bool loadedFromSnapshot);

[[nodiscard]] std::string formatInvestigationTimeline(const std::string& investigationId,
                                                      const scope::workspace::TimelineProjectionResult& result);

[[nodiscard]] std::string formatCrashReport(const scope::workspace::CrashReport& report);

[[nodiscard]] std::string formatInvestigationCrashAnalysis(const scope::workspace::CrashReport& report);

[[nodiscard]] std::string formatTailPollResult(const std::vector<std::string>& lines, bool active);

[[nodiscard]] std::string formatAnalyzeJobAccepted(const AnalyzeJobEnqueueResult& job);

} // namespace scope::web
