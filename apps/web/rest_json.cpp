/**
 * @file rest_json.cpp
 */

#include "rest_json.hpp"

#include "analytics_output.hpp"
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
        return 400;
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

std::string formatArtifactRecord(const scope::workspace::ArtifactRecord& artifact)
{
    std::ostringstream output;
    output << "{\n"
           << "  \"id\": \"" << escapeJsonString(artifact.id) << "\",\n"
           << "  \"type\": \"" << escapeJsonString(artifact.type) << "\",\n"
           << "  \"name\": \"" << escapeJsonString(artifact.name) << "\",\n"
           << "  \"relativePath\": \"" << escapeJsonString(artifact.relativePath) << "\",\n"
           << "  \"importedAt\": \"" << escapeJsonString(artifact.importedAt) << "\",\n"
           << "  \"source\": " << formatArtifactSourceJson(artifact.source) << ",\n"
           << "  \"status\": \"" << escapeJsonString(artifact.status) << "\"\n"
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

        output << "\n    " << formatArtifactRecord(manifest.artifacts[index]);
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

std::string formatInvestigationOpenResult(const std::string& investigationId, const foundation::Path& sourcePath,
                                         const scope::workspace::InvestigationSummary& summary)
{
    std::ostringstream output;
    output << "{\n"
           << "  \"opened\": true,\n"
           << "  \"investigationId\": \"" << escapeJsonString(investigationId) << "\",\n"
           << "  \"sourcePath\": \"" << escapeJsonString(sourcePath.string()) << "\",\n"
           << "  \"summary\": " << formatInvestigationSummaryJson(summary) << "\n"
           << '}';

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
