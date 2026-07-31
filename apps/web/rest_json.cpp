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

} // namespace scope::web
