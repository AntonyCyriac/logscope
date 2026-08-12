/**
 * @file web_request_parsers.cpp
 */

#include "web_request_parsers.hpp"

#include "evidence_link.hpp"
#include "correlation_suggestion.hpp"
#include "json_parse.hpp"

#include "analysis.hpp"
#include "foundation/error.hpp"
#include "foundation/timestamp.hpp"
#include "foundation/string.hpp"
#include "log_format.hpp"
#include "report_format.hpp"
#include "report_section.hpp"

#include <algorithm>
#include <filesystem>

namespace scope::web
{

namespace
{

std::optional<analysis::DetectedLogLevel> parseDetectedLogLevel(const std::string_view value)
{
    const std::string normalized = scope::foundation::trim(value);

    if (normalized == "error")
    {
        return analysis::DetectedLogLevel::Error;
    }

    if (normalized == "warning" || normalized == "warn")
    {
        return analysis::DetectedLogLevel::Warn;
    }

    if (normalized == "info")
    {
        return analysis::DetectedLogLevel::Info;
    }

    if (normalized == "other")
    {
        return analysis::DetectedLogLevel::Other;
    }

    if (normalized == "blank")
    {
        return analysis::DetectedLogLevel::Blank;
    }

    return std::nullopt;
}

analysis::LogFormat parseLogFormatHint(const std::string_view value)
{
    if (const std::optional<analysis::LogFormat> parsed = analysis::parseLogFormat(value))
    {
        return *parsed;
    }

    return analysis::LogFormat::Auto;
}

void applyStorageOverrides(analysis::AnalysisConfig& config, const std::string_view body)
{
    if (const std::optional<bool> persistIndex = jsonBoolField(body, "persistIndex"))
    {
        config.storage.persistIndex = *persistIndex;
    }

    if (const std::optional<bool> reuseIndex = jsonBoolField(body, "reuseIndex"))
    {
        config.storage.reuseIndex = *reuseIndex;
    }

    if (const std::optional<std::string> indexPath = jsonStringField(body, "indexPath"))
    {
        config.storage.indexPath = foundation::Path(*indexPath);
    }

    if (config.storage.persistIndex && config.storage.mode == storage::StorageMode::Memory)
    {
        config.storage.mode = storage::StorageMode::Hybrid;
    }
}

} // namespace

foundation::Result<foundation::Path> parsePathField(const std::string_view body)
{
    const std::optional<std::string> pathValue = jsonStringField(body, "path");

    if (!pathValue || pathValue->empty())
    {
        return foundation::Result<foundation::Path>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Missing required field: path"));
    }

    return foundation::Result<foundation::Path>(foundation::Path(*pathValue));
}

investigation::InvestigationCriteria parseInvestigationCriteria(const std::string_view body)
{
    investigation::InvestigationCriteria criteria;

    if (const std::optional<std::string> search = jsonStringField(body, "search"))
    {
        criteria.contentSearch = *search;
    }

    if (const std::optional<std::string> query = jsonStringField(body, "query"))
    {
        criteria.booleanQuery = *query;
    }

    if (const std::optional<std::string> filter = jsonStringField(body, "filter"))
    {
        criteria.filterExpression = *filter;
    }

    if (const std::optional<std::string> level = jsonStringField(body, "level"))
    {
        if (const std::optional<analysis::DetectedLogLevel> parsed = parseDetectedLogLevel(*level))
        {
            criteria.field = criteria.field.withLevel(*parsed);
        }
    }

    if (const std::optional<std::string> message = jsonStringField(body, "message"))
    {
        criteria.field = criteria.field.withMessageContains(*message);
    }

    if (const std::optional<std::string> jsonKey = jsonStringField(body, "jsonKey"))
    {
        criteria.field = criteria.field.withRequiredJsonKey(*jsonKey);
    }

    if (const std::optional<std::string> timeFrom = jsonStringField(body, "timeFrom"))
    {
        if (const auto timestamp = foundation::Timestamp::parse(*timeFrom))
        {
            criteria.timeRange = criteria.timeRange.withEarliest(timestamp.value());
        }
    }

    if (const std::optional<std::string> timeTo = jsonStringField(body, "timeTo"))
    {
        if (const auto timestamp = foundation::Timestamp::parse(*timeTo))
        {
            criteria.timeRange = criteria.timeRange.withLatest(timestamp.value());
        }
    }

    if (const std::optional<bool> regex = jsonBoolField(body, "regex"))
    {
        criteria.searchMode = *regex ? search::SearchMode::Regex : search::SearchMode::Text;
    }

    return criteria;
}

analysis::AnalysisConfig parseAnalysisConfig(const std::string_view body,
                                             const configuration::ConfigurationManager& manager)
{
    analysis::AnalysisConfig overrides;

    if (const std::optional<std::string> profile = jsonStringField(body, "profile"))
    {
        if (const std::optional<analysis::FormatProfile> resolved = analysis::resolveFormatProfile(*profile))
        {
            overrides = resolved->defaults;
        }
    }

    if (const std::optional<std::string> logFormat = jsonStringField(body, "logFormat"))
    {
        overrides.formatHint = parseLogFormatHint(*logFormat);
    }

    analysis::AnalysisConfig config =
        analysis::resolveAnalysisConfig(manager.configuration(), overrides);
    applyStorageOverrides(config, body);

    return config;
}

analytics::AnalyticsConfig parseAnalyticsConfig(const std::string_view body)
{
    analytics::AnalyticsConfig config;

    if (const std::optional<std::int64_t> bucketSeconds = jsonIntField(body, "bucketSeconds"))
    {
        config.bucketSeconds = *bucketSeconds;
    }

    if (const std::optional<std::int64_t> topN = jsonIntField(body, "topN"))
    {
        config.topN = static_cast<std::size_t>(*topN);
    }

    if (const std::optional<std::int64_t> minClusterCount = jsonIntField(body, "minClusterCount"))
    {
        config.minClusterCount = static_cast<std::uint64_t>(*minClusterCount);
    }

    if (const std::optional<bool> includeTimeline = jsonBoolField(body, "includeTimeline"))
    {
        config.includeTimeline = *includeTimeline;
    }

    return config;
}

reporting::ReportOptions parseReportOptions(const std::string_view body)
{
    reporting::ReportOptions options;
    options.format = reporting::ReportFormat::Json;

    if (const std::optional<std::string> format = jsonStringField(body, "format"))
    {
        if (const std::optional<reporting::ReportFormat> parsed = reporting::parseReportFormat(*format))
        {
            options.format = *parsed;
        }
    }

    if (const std::optional<std::string> sections = jsonStringField(body, "sections"))
    {
        if (const std::optional<reporting::ReportSections> parsed = reporting::ReportSections::parse(*sections))
        {
            options.sections = *parsed;
        }
    }

    return options;
}

AgentInvestigateRequest parseAgentInvestigateRequest(const std::string_view body)
{
    AgentInvestigateRequest request;
    request.criteria = parseInvestigationCriteria(body);

    if (const std::optional<std::string> ask = jsonStringField(body, "ask"))
    {
        request.askQuery = *ask;
    }

    if (const std::optional<bool> summarize = jsonBoolField(body, "summarize"))
    {
        request.summarize = *summarize;
    }

    if (const std::optional<bool> hints = jsonBoolField(body, "hints"))
    {
        request.hints = *hints;
    }

    return request;
}

application::SessionSaveRequest parseSessionSaveRequest(const std::string_view body)
{
    application::SessionSaveRequest request;

    if (const std::optional<std::string> sessionFile = jsonStringField(body, "sessionFile"))
    {
        request.sessionFile = foundation::Path(*sessionFile);
    }

    if (request.sessionFile.string().empty())
    {
        if (const std::optional<std::string> path = jsonStringField(body, "path"))
        {
            request.sessionFile = foundation::Path(*path);
        }
    }

    if (const std::optional<std::string> configFile = jsonStringField(body, "configFile"))
    {
        request.configFile = foundation::Path(*configFile);
    }

    request.reportOptions = parseReportOptions(body);
    request.contentCriteria = parseInvestigationCriteria(body);

    if (const std::optional<std::string> search = jsonStringField(body, "search"))
    {
        request.searchQuery = *search;
    }

    if (const std::optional<std::string> searchQuery = jsonStringField(body, "searchQuery"))
    {
        request.searchQuery = *searchQuery;
    }

    return request;
}

WorkspaceCreateRequest parseWorkspaceCreateRequest(const std::string_view body)
{
    WorkspaceCreateRequest request;

    if (const std::optional<std::string> name = jsonStringField(body, "name"))
    {
        request.name = *name;
    }

    if (const std::optional<std::string> description = jsonStringField(body, "description"))
    {
        request.description = *description;
    }

    if (const std::optional<bool> captureSession = jsonBoolField(body, "captureSession"))
    {
        request.captureSession = *captureSession;
    }

    const std::size_t sourceRefKey = body.find("\"sourceRef\"");

    if (sourceRefKey != std::string::npos)
    {
        WorkspaceSourceRef sourceRef;

        if (const std::optional<std::string> type = jsonStringField(body, "type"))
        {
            sourceRef.type = *type;
        }

        if (const std::optional<std::string> displayName = jsonStringField(body, "displayName"))
        {
            sourceRef.displayName = *displayName;
        }

        if (const std::optional<std::string> path = jsonStringField(body, "path"))
        {
            sourceRef.path = *path;
        }

        request.sourceRef = sourceRef;
    }

    return request;
}

WorkspaceUpdateRequest parseWorkspaceUpdateRequest(const std::string_view body)
{
    WorkspaceUpdateRequest request;

    if (const std::optional<std::string> name = jsonStringField(body, "name"))
    {
        request.name = *name;
    }

    if (const std::optional<std::string> description = jsonStringField(body, "description"))
    {
        request.description = *description;
    }

    return request;
}

InvestigationCreateBody parseInvestigationCreateRequest(const std::string_view body)
{
    InvestigationCreateBody request;

    if (const std::optional<std::string> name = jsonStringField(body, "name"))
    {
        request.name = *name;
    }

    if (const std::optional<std::string> description = jsonStringField(body, "description"))
    {
        request.description = *description;
    }

    if (const std::optional<bool> captureSession = jsonBoolField(body, "captureSession"))
    {
        request.captureSession = *captureSession;
    }

    return request;
}

foundation::Result<ArtifactAddRequest> parseArtifactAddRequest(const std::string_view body)
{
    ArtifactAddRequest request;

    if (const std::optional<std::string> type = jsonStringField(body, "type"))
    {
        request.type = *type;
    }

    if (request.type.empty())
    {
        return foundation::Result<ArtifactAddRequest>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Missing required field: type."));
    }

    if (request.type != "log" && request.type != "note" && request.type != "pstack" && request.type != "core")
    {
        return foundation::Result<ArtifactAddRequest>(
            foundation::Error(foundation::ErrorCode::InvalidArgument,
                                "Artifact type must be log, note, pstack, or core."));
    }

    if (const std::optional<std::string> name = jsonStringField(body, "name"))
    {
        request.name = *name;
    }

    if (const std::optional<std::string> title = jsonStringField(body, "title"))
    {
        if (request.name.empty())
        {
            request.name = *title;
        }
    }

    if (const std::optional<std::string> noteBody = jsonStringField(body, "body"))
    {
        request.body = *noteBody;
    }

    if (const std::optional<std::string> sourcePath = jsonStringField(body, "sourcePath"))
    {
        request.sourcePath = *sourcePath;
    }

    if (const std::optional<std::string> path = jsonStringField(body, "path"))
    {
        if (request.sourcePath.empty())
        {
            request.sourcePath = *path;
        }
    }

    if (const std::optional<std::string> displayName = jsonStringField(body, "displayName"))
    {
        request.displayName = *displayName;
    }

    if (const std::optional<std::string> role = jsonStringField(body, "role"))
    {
        request.role = *role;
    }

    if (request.type == "note" && request.name.empty())
    {
        return foundation::Result<ArtifactAddRequest>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Note title is required."));
    }

    if ((request.type == "pstack" || request.type == "core") && request.sourcePath.empty())
    {
        return foundation::Result<ArtifactAddRequest>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Source path is required for pstack and core artifacts."));
    }

    return foundation::Result<ArtifactAddRequest>(std::move(request));
}

InvestigationOpenRequest parseInvestigationOpenRequest(const std::string_view body)
{
    InvestigationOpenRequest request;

    if (const std::optional<std::string> artifactId = jsonStringField(body, "artifactId"))
    {
        request.artifactId = *artifactId;
    }

    return request;
}

InvestigationTimelineQuery parseInvestigationTimelineQuery(const std::string_view limitValue,
                                                           const std::string_view offsetValue,
                                                           const std::string_view orderValue)
{
    InvestigationTimelineQuery query;

    if (!limitValue.empty())
    {
        try
        {
            query.options.limit = static_cast<std::size_t>(std::stoull(std::string(limitValue)));
        }
        catch (...)
        {
        }
    }

    if (!offsetValue.empty())
    {
        try
        {
            query.options.offset = static_cast<std::size_t>(std::stoull(std::string(offsetValue)));
        }
        catch (...)
        {
        }
    }

    if (orderValue == "desc" || orderValue == "DESC")
    {
        query.options.order = scope::workspace::TimelineSortOrder::Descending;
    }

    return query;
}

InvestigationUpdateRequest parseInvestigationUpdateRequest(const std::string_view body)
{
    InvestigationUpdateRequest request;

    if (const std::optional<std::string> name = jsonStringField(body, "name"))
    {
        request.name = *name;
    }

    if (const std::optional<std::string> description = jsonStringField(body, "description"))
    {
        request.description = *description;
    }

    if (const std::optional<std::string> primaryArtifactId = jsonStringField(body, "primaryArtifactId"))
    {
        request.primaryArtifactId = *primaryArtifactId;
    }

    return request;
}

std::optional<std::string> extractJsonObjectSubstring(const std::string_view body, const std::string_view key)
{
    const std::string needle = '"' + std::string(key) + '"';
    const std::size_t keyPosition = body.find(needle);

    if (keyPosition == std::string::npos)
    {
        return std::nullopt;
    }

    const std::size_t openBrace = body.find('{', keyPosition);

    if (openBrace == std::string::npos)
    {
        return std::nullopt;
    }

    std::size_t depth = 0U;

    for (std::size_t index = openBrace; index < body.size(); ++index)
    {
        if (body[index] == '{')
        {
            ++depth;
        }
        else if (body[index] == '}')
        {
            --depth;

            if (depth == 0U)
            {
                return std::string(body.substr(openBrace, index - openBrace + 1U));
            }
        }
    }

    return std::nullopt;
}

scope::workspace::LinkEndpoint parseLinkEndpointFromBody(const std::string_view objectBody)
{
    scope::workspace::LinkEndpoint endpoint;
    const std::optional<std::string> kind = jsonStringField(objectBody, "kind");
    const std::optional<std::string> eventId = jsonStringField(objectBody, "eventId");

    if (kind)
    {
        endpoint.kind = *kind;
    }

    if (eventId)
    {
        endpoint.eventId = *eventId;
    }

    return endpoint;
}

foundation::Result<scope::workspace::EvidenceLinkCreateRequest> parseEvidenceLinkCreateRequest(
    const std::string_view body)
{
    scope::workspace::EvidenceLinkCreateRequest request;

    const std::optional<std::string> typeValue = jsonStringField(body, "type");

    if (!typeValue)
    {
        return foundation::Result<scope::workspace::EvidenceLinkCreateRequest>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Evidence link type is required."));
    }

    const std::optional<scope::workspace::EvidenceLinkType> parsedType =
        scope::workspace::parseEvidenceLinkType(*typeValue);

    if (!parsedType)
    {
        return foundation::Result<scope::workspace::EvidenceLinkCreateRequest>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Invalid evidence link type."));
    }

    request.type = *parsedType;

    const std::optional<std::string> sourceObject = extractJsonObjectSubstring(body, "source");

    if (!sourceObject)
    {
        return foundation::Result<scope::workspace::EvidenceLinkCreateRequest>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Evidence link source is required."));
    }

    request.source = parseLinkEndpointFromBody(*sourceObject);

    const std::optional<std::string> targetObject = extractJsonObjectSubstring(body, "target");

    if (!targetObject)
    {
        return foundation::Result<scope::workspace::EvidenceLinkCreateRequest>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Evidence link target is required."));
    }

    request.target = parseLinkEndpointFromBody(*targetObject);

    if (const std::optional<std::string> note = jsonStringField(body, "note"))
    {
        request.note = *note;
    }

    return foundation::Result<scope::workspace::EvidenceLinkCreateRequest>(std::move(request));
}

scope::workspace::CorrelationSuggestionQuery parseCorrelationSuggestionQuery(const std::string_view eventIdValue,
                                                                             const std::string_view limitValue,
                                                                             const std::string_view offsetValue)
{
    scope::workspace::CorrelationSuggestionQuery query;

    if (!eventIdValue.empty())
    {
        query.eventId = std::string(eventIdValue);
    }

    if (!limitValue.empty())
    {
        try
        {
            query.limit = std::stoi(std::string(limitValue));
        }
        catch (...)
        {
            query.limit = -1;
        }
    }

    if (!offsetValue.empty())
    {
        try
        {
            query.offset = std::stoi(std::string(offsetValue));
        }
        catch (...)
        {
            query.offset = -1;
        }
    }

    return query;
}

foundation::Result<scope::workspace::EvidenceLinkType> parseOptionalCorrelationAcceptType(
    const std::string_view body)
{
    if (body.empty())
    {
        return foundation::Result<scope::workspace::EvidenceLinkType>(scope::workspace::EvidenceLinkType::Related);
    }

    const std::optional<std::string> typeValue = jsonStringField(body, "type");

    if (!typeValue || typeValue->empty())
    {
        return foundation::Result<scope::workspace::EvidenceLinkType>(scope::workspace::EvidenceLinkType::Related);
    }

    const std::optional<scope::workspace::EvidenceLinkType> parsedType =
        scope::workspace::parseEvidenceLinkType(*typeValue);

    if (!parsedType)
    {
        return foundation::Result<scope::workspace::EvidenceLinkType>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Invalid evidence link type."));
    }

    return foundation::Result<scope::workspace::EvidenceLinkType>(*parsedType);
}

std::optional<std::string> parseOptionalCorrelationAcceptNote(const std::string_view body)
{
    if (body.empty())
    {
        return std::nullopt;
    }

    return jsonStringField(body, "note");
}

foundation::Result<bool> validateServerPath(const WebConfig& config, const foundation::Path& path)
{
    if (!config.allowServerPaths)
    {
        return foundation::Result<bool>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Server path open is disabled (web.allow_server_paths=false)."));
    }

    const std::string pathString = path.string();

    if (pathString.find("..") != std::string::npos || pathString.find('\0') != std::string::npos)
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Invalid path: traversal rejected."));
    }

    std::error_code errorCode;
    const std::filesystem::path absolutePath = std::filesystem::weakly_canonical(pathString, errorCode);

    if (errorCode)
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Invalid path: could not resolve."));
    }

    if (config.allowedPathRoots.empty())
    {
        return foundation::Result<bool>(foundation::Error(
            foundation::ErrorCode::InvalidArgument,
            "web.allowed_path_roots must be configured when web.allow_server_paths is enabled."));
    }

    const std::string absoluteString = absolutePath.string();

    for (const foundation::Path& root : config.allowedPathRoots)
    {
        std::error_code rootError;
        const std::filesystem::path absoluteRoot = std::filesystem::weakly_canonical(root.string(), rootError);

        if (rootError)
        {
            continue;
        }

        const std::string rootString = absoluteRoot.string();

        if (absoluteString.rfind(rootString, 0) == 0)
        {
            return foundation::Result<bool>(true);
        }
    }

    return foundation::Result<bool>(foundation::Error(
        foundation::ErrorCode::InvalidArgument, "Path is outside allowed roots (web.allowed_path_roots)."));
}

namespace
{

foundation::Path effectiveUploadTempDir(const WebConfig& config)
{
    if (!config.uploadTempDir.string().empty())
    {
        return config.uploadTempDir;
    }

    return foundation::Path(std::filesystem::temp_directory_path().string());
}

bool isPathUnderRoot(const foundation::Path& path, const foundation::Path& root)
{
    std::error_code pathError;
    const std::filesystem::path absolutePath = std::filesystem::weakly_canonical(path.string(), pathError);

    if (pathError)
    {
        return false;
    }

    std::error_code rootError;
    const std::filesystem::path absoluteRoot = std::filesystem::weakly_canonical(root.string(), rootError);

    if (rootError)
    {
        return false;
    }

    const std::string absoluteString = absolutePath.string();
    const std::string rootString = absoluteRoot.string();

    return absoluteString.rfind(rootString, 0) == 0;
}

} // namespace

foundation::Result<bool> validateArtifactSourcePath(const WebConfig& config, const foundation::Path& path)
{
    const auto allowedRootResult = validateServerPath(config, path);

    if (allowedRootResult)
    {
        return allowedRootResult;
    }

    if (isPathUnderRoot(path, effectiveUploadTempDir(config)))
    {
        return foundation::Result<bool>(true);
    }

    return allowedRootResult;
}

} // namespace scope::web
