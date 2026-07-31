/**
 * @file web_request_parsers.cpp
 */

#include "web_request_parsers.hpp"

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

    return request;
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

} // namespace scope::web
