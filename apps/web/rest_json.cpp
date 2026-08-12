/**
 * @file rest_json.cpp
 */

#include "rest_json.hpp"

#include "analytics_output.hpp"
#include "crash_report.hpp"
#include "correlation_suggestion.hpp"
#include "evidence_link.hpp"
#include "investigation_output.hpp"
#include "output_format.hpp"
#include "report_format.hpp"
#include "report_options.hpp"
#include "report_output.hpp"

#include <sstream>

namespace scope::web
{

namespace
{

std::string errorCodeToken(const foundation::ErrorCode code)
{
    switch (code)
    {
    case foundation::ErrorCode::InvalidArgument:
        return "INVALID_ARGUMENT";
    case foundation::ErrorCode::FileNotFound:
        return "NOT_FOUND";
    case foundation::ErrorCode::IOError:
        return "INTERNAL";
    case foundation::ErrorCode::ParseError:
        return "INVALID_ARGUMENT";
    case foundation::ErrorCode::InvalidLinkTarget:
        return "INVALID_LINK_TARGET";
    case foundation::ErrorCode::DuplicateEvidenceLink:
        return "DUPLICATE_EVIDENCE_LINK";
    case foundation::ErrorCode::StaleSuggestion:
        return "STALE_SUGGESTION";
    case foundation::ErrorCode::Unknown:
        return "INTERNAL";
    case foundation::ErrorCode::None:
        return "INTERNAL";
    }

    return "INTERNAL";
}

std::string extensionStatusName(const extension::ExtensionStatus status)
{
    switch (status)
    {
    case extension::ExtensionStatus::Ready:
        return "ready";
    case extension::ExtensionStatus::Disabled:
        return "disabled";
    case extension::ExtensionStatus::InitializationFailed:
        return "failed";
    }

    return "unknown";
}

} // namespace

std::string escapeJsonString(const std::string_view value)
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

std::string successEnvelope(const std::string_view dataJson)
{
    std::ostringstream output;
    output << "{\"data\": ";

    if (dataJson.empty())
    {
        output << "null";
    }
    else
    {
        output << dataJson;
    }

    output << '}';

    return output.str();
}

std::string errorEnvelope(const std::string_view code, const std::string_view message,
                          const std::string_view detailsJson)
{
    return std::string("{\"error\":{\"code\":\"") + escapeJsonString(code) + "\",\"message\":\"" +
           escapeJsonString(message) + "\",\"details\":" + std::string(detailsJson) + "}}";
}

std::string errorEnvelopeFromFoundation(const foundation::Error& error)
{
    return errorEnvelope(errorCodeToken(error.code()), error.message());
}

int httpStatusForError(const foundation::Error& error)
{
    switch (error.code())
    {
    case foundation::ErrorCode::InvalidArgument:
    case foundation::ErrorCode::ParseError:
    case foundation::ErrorCode::InvalidLinkTarget:
    case foundation::ErrorCode::StaleSuggestion:
        return 400;
    case foundation::ErrorCode::DuplicateEvidenceLink:
        return 409;
    case foundation::ErrorCode::FileNotFound:
        return 404;
    case foundation::ErrorCode::IOError:
    case foundation::ErrorCode::Unknown:
        return 500;
    case foundation::ErrorCode::None:
        return 500;
    }

    return 500;
}

std::string formatExtensionInfo(const extension::ExtensionInfo& info)
{
    std::ostringstream output;

    output << "{\n"
           << "  \"id\": \"" << escapeJsonString(info.id) << "\",\n"
           << "  \"version\": \"" << escapeJsonString(info.version) << "\",\n"
           << "  \"description\": \"" << escapeJsonString(info.description) << "\",\n"
           << "  \"enabled\": " << (info.enabled ? "true" : "false") << ",\n"
           << "  \"status\": \"" << extensionStatusName(info.status) << "\",\n"
           << "  \"dynamic\": " << (info.dynamic ? "true" : "false") << ",\n"
           << "  \"apiVersion\": " << info.apiVersion;

    if (!info.libraryPath.empty())
    {
        output << ",\n  \"libraryPath\": \"" << escapeJsonString(info.libraryPath) << '"';
    }

    output << "\n}";

    return output.str();
}

std::string formatExtensionList(const std::vector<extension::ExtensionInfo>& extensions)
{
    std::ostringstream output;
    output << '[';

    for (std::size_t index = 0U; index < extensions.size(); ++index)
    {
        if (index > 0U)
        {
            output << ',';
        }

        output << formatExtensionInfo(extensions[index]);
    }

    output << ']';

    return output.str();
}

std::string formatPathList(const std::vector<foundation::Path>& paths)
{
    std::ostringstream output;
    output << '[';

    for (std::size_t index = 0U; index < paths.size(); ++index)
    {
        if (index > 0U)
        {
            output << ',';
        }

        output << '"' << escapeJsonString(paths[index].string()) << '"';
    }

    output << ']';

    return output.str();
}

std::string formatAnalyticsJson(const analytics::AnalyticsResult& result)
{
    std::ostringstream stream;
    scope::cli::writeAnalyticsOutput(result, scope::cli::OutputFormat::Json, stream);

    return stream.str();
}

std::string formatAnalyzeJson(const analysis::AnalysisModel& model)
{
    reporting::ReportOptions options;
    options.format = reporting::ReportFormat::Json;
    const reporting::Report report = scope::cli::generateAnalysisReport(model, options);

    return report.text();
}

std::string formatInvestigationJson(const investigation::InvestigationResult& result)
{
    return scope::cli::formatInvestigationOutput(result, scope::cli::OutputFormat::Json);
}

std::string formatAgentInvestigateJson(const application::AgentInvestigateResult& result)
{
    std::ostringstream output;
    output << "{\n"
           << "  \"investigation\": " << formatInvestigationJson(result.investigation) << ",\n"
           << "  \"aiErrors\": [";

    for (std::size_t index = 0U; index < result.aiErrors.size(); ++index)
    {
        if (index > 0U)
        {
            output << ',';
        }

        output << '"' << escapeJsonString(result.aiErrors[index]) << '"';
    }

    output << "]\n}";

    return output.str();
}

namespace
{

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

std::string formatInvestigationSummaryJson(const scope::workspace::InvestigationSummary& summary)
{
    std::ostringstream output;
    output << "{\n"
           << "    \"hasModel\": " << (summary.hasModel ? "true" : "false") << ",\n"
           << "    \"lineCount\": " << summary.lineCount << ",\n"
           << "    \"errorCount\": " << summary.errorCount << "\n"
           << "  }";

    return output.str();
}

std::string formatArtifactSourceJson(const scope::workspace::ArtifactSource& source)
{
    std::ostringstream output;
    output << "{\n"
           << "      \"kind\": \"" << escapeJsonString(source.kind) << "\",\n"
           << "      \"displayName\": \"" << escapeJsonString(source.displayName) << "\"\n"
           << "    }";

    return output.str();
}

} // namespace

std::string formatWorkspaceMetadata(const WorkspaceMetadata& metadata)
{
    std::ostringstream output;
    output << "{\n"
           << "  \"id\": \"" << escapeJsonString(metadata.id) << "\",\n"
           << "  \"name\": \"" << escapeJsonString(metadata.name) << "\",\n"
           << "  \"description\": \"" << escapeJsonString(metadata.description) << "\",\n"
           << "  \"createdAt\": \"" << escapeJsonString(metadata.createdAt) << "\",\n"
           << "  \"updatedAt\": \"" << escapeJsonString(metadata.updatedAt) << "\",\n"
           << "  \"sourceRef\": " << formatSourceRefJson(metadata.sourceRef) << ",\n"
           << "  \"summary\": " << formatSummaryJson(metadata.summary) << "\n"
           << '}';

    return output.str();
}

std::string formatWorkspaceList(const WorkspaceListResult& list)
{
    std::ostringstream output;
    output << "{\n  \"workspaces\": [";

    for (std::size_t index = 0U; index < list.workspaces.size(); ++index)
    {
        if (index > 0U)
        {
            output << ',';
        }

        const WorkspaceMetadata& metadata = list.workspaces[index];
        output << "\n    {\n"
               << "      \"id\": \"" << escapeJsonString(metadata.id) << "\",\n"
               << "      \"name\": \"" << escapeJsonString(metadata.name) << "\",\n"
               << "      \"updatedAt\": \"" << escapeJsonString(metadata.updatedAt) << "\",\n"
               << "      \"summary\": " << formatSummaryJson(metadata.summary) << "\n"
               << "    }";
    }

    output << "\n  ],\n  \"truncated\": " << (list.truncated ? "true" : "false") << "\n}";

    return output.str();
}

std::string formatWorkspaceOpenResult(const std::string& workspaceId, const foundation::Path& sourcePath,
                                      const WorkspaceSummary& summary)
{
    std::ostringstream output;
    output << "{\n"
           << "  \"opened\": true,\n"
           << "  \"workspaceId\": \"" << escapeJsonString(workspaceId) << "\",\n"
           << "  \"sourcePath\": \"" << escapeJsonString(sourcePath.string()) << "\",\n"
           << "  \"summary\": " << formatSummaryJson(summary) << "\n"
           << '}';

    return output.str();
}

std::string formatStringMapJson(const std::map<std::string, std::string>& values)
{
    std::ostringstream output;
    output << '{';
    bool first = true;

    for (const auto& entry : values)
    {
        if (!first)
        {
            output << ',';
        }

        first = false;
        output << "\n    \"" << escapeJsonString(entry.first) << "\": \"" << escapeJsonString(entry.second) << '"';
    }

    if (!first)
    {
        output << '\n';
    }

    output << '}';

    return output.str();
}

std::string formatArtifactRecord(const scope::workspace::ArtifactRecord& artifact,
                                 const std::string& primaryArtifactId)
{
    std::ostringstream output;
    output << "{\n"
           << "  \"id\": \"" << escapeJsonString(artifact.id) << "\",\n"
           << "  \"type\": \"" << escapeJsonString(artifact.type) << "\",\n"
           << "  \"name\": \"" << escapeJsonString(artifact.name) << "\",\n"
           << "  \"relativePath\": \"" << escapeJsonString(artifact.relativePath) << "\",\n"
           << "  \"importedAt\": \"" << escapeJsonString(artifact.importedAt) << "\",\n"
           << "  \"source\": " << formatArtifactSourceJson(artifact.source) << ",\n"
           << "  \"status\": \"" << escapeJsonString(artifact.status) << "\",\n"
           << "  \"metadata\": " << formatStringMapJson(artifact.metadata) << ",\n"
           << "  \"isEntry\": " << ((!primaryArtifactId.empty() && artifact.id == primaryArtifactId) ? "true" : "false")
           << "\n"
           << '}';

    return output.str();
}

std::string formatInvestigationManifest(const scope::workspace::InvestigationManifest& manifest)
{
    std::ostringstream output;
    output << "{\n"
           << "  \"schemaVersion\": " << manifest.schemaVersion << ",\n"
           << "  \"id\": \"" << escapeJsonString(manifest.id) << "\",\n"
           << "  \"name\": \"" << escapeJsonString(manifest.name) << "\",\n"
           << "  \"description\": \"" << escapeJsonString(manifest.description) << "\",\n"
           << "  \"createdAt\": \"" << escapeJsonString(manifest.createdAt) << "\",\n"
           << "  \"updatedAt\": \"" << escapeJsonString(manifest.updatedAt) << "\",\n"
           << "  \"primaryArtifactId\": \"" << escapeJsonString(manifest.primaryArtifactId) << "\",\n"
           << "  \"summary\": " << formatInvestigationSummaryJson(manifest.summary) << ",\n"
           << "  \"snapshotFile\": \"" << escapeJsonString(manifest.snapshotFile) << "\",\n"
           << "  \"artifacts\": [";

    for (std::size_t index = 0U; index < manifest.artifacts.size(); ++index)
    {
        if (index > 0U)
        {
            output << ',';
        }

        output << "\n    " << formatArtifactRecord(manifest.artifacts[index], manifest.primaryArtifactId);
    }

    output << "\n  ]\n}";

    return output.str();
}

std::string formatInvestigationList(const InvestigationListResult& list)
{
    std::ostringstream output;
    output << "{\n  \"investigations\": [";

    for (std::size_t index = 0U; index < list.investigations.size(); ++index)
    {
        if (index > 0U)
        {
            output << ',';
        }

        const scope::workspace::InvestigationManifest& manifest = list.investigations[index];
        output << "\n    {\n"
               << "      \"id\": \"" << escapeJsonString(manifest.id) << "\",\n"
               << "      \"name\": \"" << escapeJsonString(manifest.name) << "\",\n"
               << "      \"updatedAt\": \"" << escapeJsonString(manifest.updatedAt) << "\",\n"
               << "      \"summary\": " << formatInvestigationSummaryJson(manifest.summary) << ",\n"
               << "      \"artifactCount\": " << manifest.artifacts.size() << "\n"
               << "    }";
    }

    output << "\n  ],\n  \"truncated\": " << (list.truncated ? "true" : "false") << "\n}";

    return output.str();
}

std::string formatInvestigationOpenResult(const std::string& investigationId, const std::string& artifactId,
                                         const std::string& artifactType, const foundation::Path& sourcePath,
                                         const scope::workspace::InvestigationSummary& summary,
                                         const bool loadedFromSnapshot)
{
    std::ostringstream output;
    output << "{\n"
           << "  \"opened\": true,\n"
           << "  \"investigationId\": \"" << escapeJsonString(investigationId) << "\",\n"
           << "  \"artifactId\": \"" << escapeJsonString(artifactId) << "\",\n"
           << "  \"artifactType\": \"" << escapeJsonString(artifactType) << "\",\n"
           << "  \"sourcePath\": \"" << escapeJsonString(sourcePath.string()) << "\",\n"
           << "  \"loadedFromSnapshot\": " << (loadedFromSnapshot ? "true" : "false") << ",\n"
           << "  \"summary\": " << formatInvestigationSummaryJson(summary) << "\n"
           << '}';

    return output.str();
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

std::string formatTimelineEventJson(const scope::workspace::TimelineEvent& event)
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

std::string formatInvestigationTimeline(const std::string& investigationId,
                                        const scope::workspace::TimelineProjectionResult& result)
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

std::string formatCrashFrameJson(const scope::workspace::CrashFrame& frame)
{
    std::ostringstream output;
    output << "{\n"
           << "  \"index\": " << frame.index << ",\n"
           << "  \"address\": \"" << escapeJsonString(frame.address) << "\",\n"
           << "  \"symbol\": \"" << escapeJsonString(frame.symbol) << "\"";

    if (frame.module.has_value())
    {
        output << ",\n  \"module\": \"" << escapeJsonString(*frame.module) << '"';
    }

    if (frame.location.has_value())
    {
        output << ",\n  \"location\": \"" << escapeJsonString(*frame.location) << '"';
    }

    output << "\n}";

    return output.str();
}

std::string formatCrashThreadJson(const scope::workspace::CrashThread& thread)
{
    std::ostringstream output;
    output << "{\n"
           << "  \"id\": \"" << escapeJsonString(thread.id) << "\",\n"
           << "  \"name\": \"" << escapeJsonString(thread.name) << "\",\n"
           << "  \"isFaultThread\": " << (thread.isFaultThread ? "true" : "false") << ",\n"
           << "  \"frames\": [";

    for (std::size_t index = 0U; index < thread.frames.size(); ++index)
    {
        if (index > 0U)
        {
            output << ',';
        }

        output << "\n    " << formatCrashFrameJson(thread.frames[index]);
    }

    output << "\n  ]\n}";

    return output.str();
}

std::string formatStringArrayJson(const std::vector<std::string>& values)
{
    std::ostringstream output;
    output << '[';

    for (std::size_t index = 0U; index < values.size(); ++index)
    {
        if (index > 0U)
        {
            output << ',';
        }

        output << "\n    \"" << escapeJsonString(values[index]) << '"';
    }

    if (!values.empty())
    {
        output << '\n';
    }

    output << "  ]";

    return output.str();
}

std::string formatCrashReport(const scope::workspace::CrashReport& report)
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

    for (std::size_t index = 0U; index < report.threads.size(); ++index)
    {
        if (index > 0U)
        {
            output << ',';
        }

        output << "\n    " << formatCrashThreadJson(report.threads[index]);
    }

    output << "\n  ],\n"
           << "  \"observations\": " << formatStringArrayJson(report.observations) << ",\n"
           << "  \"warnings\": " << formatStringArrayJson(report.warnings) << ",\n"
           << "  \"metadata\": " << formatStringMapJson(report.metadata);

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

std::string formatInvestigationCrashAnalysis(const scope::workspace::CrashReport& report)
{
    return std::string("{\n  \"report\": ") + formatCrashReport(report) + "\n}";
}

std::string formatLinkEndpointJson(const scope::workspace::LinkEndpoint& endpoint)
{
    std::ostringstream output;
    output << "{\n"
           << "        \"kind\": \"" << escapeJsonString(endpoint.kind) << "\",\n"
           << "        \"eventId\": \"" << escapeJsonString(endpoint.eventId) << "\"\n"
           << "      }";

    return output.str();
}

std::string formatEvidenceLinkRecord(const scope::workspace::EvidenceLinkRecord& link)
{
    std::ostringstream output;
    output << "{\n"
           << "        \"id\": \"" << escapeJsonString(link.id) << "\",\n"
           << "        \"type\": \"" << escapeJsonString(evidenceLinkTypeToString(link.type)) << "\",\n"
           << "        \"source\": " << formatLinkEndpointJson(link.source) << ",\n"
           << "        \"target\": " << formatLinkEndpointJson(link.target) << ",\n"
           << "        \"createdAt\": \"" << escapeJsonString(link.createdAt) << "\",\n"
           << "        \"status\": \"" << escapeJsonString(evidenceLinkStatusToString(link.status)) << "\"";

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

std::string formatEvidenceLinksList(const std::string& investigationId,
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

        output << "\n      " << formatEvidenceLinkRecord(links[index]);
    }

    if (!links.empty())
    {
        output << '\n';
    }

    output << "    ]\n}";

    return output.str();
}

std::string formatCorrelationSuggestion(const scope::workspace::CorrelationSuggestion& suggestion)
{
    std::ostringstream output;
    output << "{\n"
           << "        \"id\": \"" << escapeJsonString(suggestion.id) << "\",\n"
           << "        \"sourceEventId\": \"" << escapeJsonString(suggestion.sourceEventId) << "\",\n"
           << "        \"targetEventId\": \"" << escapeJsonString(suggestion.targetEventId) << "\",\n"
           << "        \"matchedKey\": \"" << escapeJsonString(correlationKeyToString(suggestion.matchedKey))
           << "\",\n"
           << "        \"matchedValue\": \"" << escapeJsonString(suggestion.matchedValue) << "\",\n"
           << "        \"sourceArtifactName\": \"" << escapeJsonString(suggestion.sourceArtifactName) << "\",\n"
           << "        \"targetArtifactName\": \"" << escapeJsonString(suggestion.targetArtifactName) << "\",\n"
           << "        \"sourceLineRef\": "
           << (suggestion.sourceLineRef.has_value() ? std::to_string(*suggestion.sourceLineRef) : "null") << ",\n"
           << "        \"targetLineRef\": "
           << (suggestion.targetLineRef.has_value() ? std::to_string(*suggestion.targetLineRef) : "null") << ",\n"
           << "        \"timeDeltaMs\": "
           << (suggestion.timeDeltaMs.has_value() ? std::to_string(*suggestion.timeDeltaMs) : "null") << ",\n"
           << "        \"ruleId\": \"" << escapeJsonString(suggestion.ruleId) << "\",\n"
           << "        \"summary\": \"" << escapeJsonString(suggestion.summary) << "\"\n"
           << "      }";

    return output.str();
}

std::string formatCorrelationSuggestionsList(const std::string& investigationId,
                                             const scope::workspace::CorrelationSuggestionListResult& result)
{
    std::ostringstream output;
    output << "{\n"
           << "  \"investigationId\": \"" << escapeJsonString(investigationId) << "\",\n"
           << "  \"suggestions\": [";

    for (std::size_t index = 0U; index < result.suggestions.size(); ++index)
    {
        if (index > 0U)
        {
            output << ',';
        }

        output << "\n      " << formatCorrelationSuggestion(result.suggestions[index]);
    }

    if (!result.suggestions.empty())
    {
        output << '\n';
    }

    output << "    ],\n"
           << "  \"total\": " << result.total << ",\n"
           << "  \"limit\": " << result.limit << ",\n"
           << "  \"offset\": " << result.offset << ",\n"
           << "  \"truncated\": " << (result.truncated ? "true" : "false") << "\n}";

    return output.str();
}

std::string formatTailPollResult(const std::vector<std::string>& lines, const bool active)
{
    std::ostringstream output;
    output << "{\n  \"lines\": [";

    for (std::size_t index = 0U; index < lines.size(); ++index)
    {
        if (index > 0U)
        {
            output << ',';
        }

        output << "\n    \"" << escapeJsonString(lines[index]) << '"';
    }

    output << "\n  ],\n  \"active\": " << (active ? "true" : "false") << "\n}";

    return output.str();
}

std::string formatAnalyzeJobAccepted(const AnalyzeJobEnqueueResult& job)
{
    std::ostringstream output;
    output << "{\n"
           << "  \"jobId\": \"" << escapeJsonString(job.jobId) << "\",\n"
           << "  \"status\": \"running\",\n"
           << "  \"pollUrl\": \"" << escapeJsonString(job.pollUrl) << "\"\n"
           << '}';

    return output.str();
}

} // namespace scope::web
