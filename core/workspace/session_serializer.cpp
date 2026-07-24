/**
 * @file session_serializer.cpp
 * @brief SessionSerializer implementation.
 */

#include "session_serializer.hpp"

#include <fstream>
#include <sstream>
#include <charconv>

#include "foundation/error.hpp"
#include "foundation/string.hpp"
#include "investigation_criteria.hpp"
#include "log_format.hpp"
#include "log_line_classifier.hpp"
#include "report_format.hpp"
#include "foundation/string.hpp"
#include "search_query_parser.hpp"

namespace scope::workspace
{

namespace
{

constexpr std::string_view sessionVersion = "1.2";

bool parseUint64(std::string_view value, std::uint64_t& output) noexcept
{
    if (value.empty())
    {
        return false;
    }

    const auto result = std::from_chars(value.data(), value.data() + value.size(), output);

    return result.ec == std::errc{} && result.ptr == value.data() + value.size();
}

bool assignUint64Field(std::string_view value, std::uint64_t& field)
{
    if (value.empty())
    {
        field = 0U;
        return true;
    }

    return parseUint64(value, field);
}

std::string sectionsToString(const reporting::ReportSections& sections)
{
    if (sections.includes(reporting::ReportSection::ExecutiveSummary) &&
        sections.includes(reporting::ReportSection::Summary) &&
        sections.includes(reporting::ReportSection::LevelBreakdown) &&
        sections.includes(reporting::ReportSection::ErrorSummary) &&
        sections.includes(reporting::ReportSection::Charts) &&
        sections.includes(reporting::ReportSection::SourceMetadata) &&
        sections.includes(reporting::ReportSection::FormatsFooter))
    {
        return "all";
    }

    std::ostringstream output;
    bool wroteSection = false;

    auto appendSection = [&](const char* name) {
        if (wroteSection)
        {
            output << ',';
        }

        output << name;
        wroteSection = true;
    };

    if (sections.includes(reporting::ReportSection::ExecutiveSummary))
    {
        appendSection("executive");
    }

    if (sections.includes(reporting::ReportSection::Summary))
    {
        appendSection("summary");
    }

    if (sections.includes(reporting::ReportSection::LevelBreakdown))
    {
        appendSection("levels");
    }

    if (sections.includes(reporting::ReportSection::ErrorSummary))
    {
        appendSection("errors");
    }

    if (sections.includes(reporting::ReportSection::Charts))
    {
        appendSection("charts");
    }

    if (sections.includes(reporting::ReportSection::SourceMetadata))
    {
        appendSection("metadata");
    }

    if (sections.includes(reporting::ReportSection::FormatsFooter))
    {
        appendSection("formats");
    }

    return output.str();
}

reporting::ReportSections sectionsFromString(const std::string& value)
{
    const auto parsed = reporting::ReportSections::parse(value);

    if (parsed)
    {
        return *parsed;
    }

    return reporting::ReportSections::all();
}

investigation::LineCountFilter lineFilterFromValues(const std::uint64_t minLines, const std::string& maxLinesValue)
{
    investigation::LineCountFilter filter = investigation::LineCountFilter::any().withMinimum(minLines);

    if (!foundation::isBlank(maxLinesValue))
    {
        filter = filter.withMaximum(std::stoull(maxLinesValue));
    }

    return filter;
}

std::string detectedLevelName(const analysis::DetectedLogLevel level)
{
    switch (level)
    {
    case analysis::DetectedLogLevel::Error:
        return "error";
    case analysis::DetectedLogLevel::Warn:
        return "warning";
    case analysis::DetectedLogLevel::Info:
        return "info";
    case analysis::DetectedLogLevel::Blank:
        return "blank";
    case analysis::DetectedLogLevel::Other:
        return "other";
    }

    return "other";
}

std::optional<analysis::DetectedLogLevel> parseDetectedLevelName(const std::string& value)
{
    const std::string lowered = foundation::toLower(value);

    if (lowered == "error")
    {
        return analysis::DetectedLogLevel::Error;
    }

    if (lowered == "warn" || lowered == "warning")
    {
        return analysis::DetectedLogLevel::Warn;
    }

    if (lowered == "info")
    {
        return analysis::DetectedLogLevel::Info;
    }

    if (lowered == "blank")
    {
        return analysis::DetectedLogLevel::Blank;
    }

    if (lowered == "other")
    {
        return analysis::DetectedLogLevel::Other;
    }

    return std::nullopt;
}

void appendContentCriteria(std::ostringstream& output, const investigation::InvestigationCriteria& criteria)
{
    output << "filter.contentSearch=" << criteria.contentSearch << '\n';

    if (criteria.timeRange.earliest().has_value())
    {
        output << "filter.timeFrom=" << criteria.timeRange.earliest()->toString() << '\n';
    }
    else
    {
        output << "filter.timeFrom=\n";
    }

    if (criteria.timeRange.latest().has_value())
    {
        output << "filter.timeTo=" << criteria.timeRange.latest()->toString() << '\n';
    }
    else
    {
        output << "filter.timeTo=\n";
    }

    if (criteria.field.level().has_value())
    {
        output << "filter.lineLevel=" << detectedLevelName(*criteria.field.level()) << '\n';
    }
    else
    {
        output << "filter.lineLevel=\n";
    }

    output << "filter.messageContains=" << criteria.field.messageContains() << '\n';
    output << "filter.jsonKey=" << criteria.field.requiredJsonKey() << '\n';

    if (!criteria.booleanQuery.empty())
    {
        output << "search.query=" << criteria.booleanQuery << '\n';
    }
    else if (criteria.searchQuery.has_value())
    {
        output << "search.query=" << criteria.searchQuery->toString() << '\n';
    }
    else if (!criteria.contentSearch.empty())
    {
        output << "search.query=" << criteria.contentSearch << '\n';
    }
    else
    {
        output << "search.query=\n";
    }
}

investigation::InvestigationCriteria contentCriteriaFromValues(
    const std::string& contentSearch, const std::string& timeFromValue, const std::string& timeToValue,
    const std::string& lineLevelValue, const std::string& messageContains, const std::string& jsonKey)
{
    investigation::InvestigationCriteria criteria;
    criteria.contentSearch = contentSearch;

    if (!foundation::isBlank(timeFromValue))
    {
        const auto earliest = foundation::Timestamp::parse(timeFromValue);

        if (earliest.hasValue())
        {
            criteria.timeRange = criteria.timeRange.withEarliest(*earliest);
        }
    }

    if (!foundation::isBlank(timeToValue))
    {
        const auto latest = foundation::Timestamp::parse(timeToValue);

        if (latest.hasValue())
        {
            criteria.timeRange = criteria.timeRange.withLatest(*latest);
        }
    }

    if (!foundation::isBlank(lineLevelValue))
    {
        const auto level = parseDetectedLevelName(lineLevelValue);

        if (level)
        {
            criteria.field = criteria.field.withLevel(*level);
        }
    }

    if (!messageContains.empty())
    {
        criteria.field = criteria.field.withMessageContains(messageContains);
    }

    if (!jsonKey.empty())
    {
        criteria.field = criteria.field.withRequiredJsonKey(jsonKey);
    }

    return criteria;
}

} // namespace

std::string SessionSerializer::serialize(const InvestigationSession& session)
{
    const analysis::AnalysisModel& model = session.analysisModel();
    const analysis::LogLevelCounts& levels = model.levelCounts();

    std::ostringstream output;

    output << "# LogScope investigation session\n";
    output << "session.version=" << sessionVersion << '\n';
    output << "session.id=" << session.sessionId().toString() << '\n';
    output << "source.path=" << session.sourcePath().string() << '\n';
    output << "analysis.totalLines=" << model.totalLines() << '\n';
    output << "analysis.format=" << analysis::logFormatName(model.format()) << '\n';
    output << "analysis.infoLines=" << levels.infoLines() << '\n';
    output << "analysis.warningLines=" << levels.warnLines() << '\n';
    output << "analysis.errorLines=" << levels.errorLines() << '\n';
    output << "analysis.otherLines=" << levels.otherLines() << '\n';
    output << "analysis.blankLines=" << levels.blankLines() << '\n';
    output << "filter.minLines=" << session.lineFilter().minimumLines() << '\n';

    if (session.lineFilter().hasMaximumLines())
    {
        output << "filter.maxLines=" << session.lineFilter().maximumLines() << '\n';
    }
    else
    {
        output << "filter.maxLines=\n";
    }

    output << "filter.minErrors=" << session.levelFilter().minimumErrors() << '\n';
    output << "filter.minWarnings=" << session.levelFilter().minimumWarnings() << '\n';
    output << "filter.searchQuery=" << session.searchQuery() << '\n';
    appendContentCriteria(output, session.contentCriteria());
    output << "search.history=" << session.searchHistory().serialize() << '\n';
    output << "report.format=" << reporting::reportFormatName(session.reportOptions().format) << '\n';
    output << "report.sections=" << sectionsToString(session.reportOptions().sections) << '\n';
    output << "config.path=" << session.configFile().string() << '\n';

    return output.str();
}

foundation::Result<InvestigationSession> SessionSerializer::deserialize(const std::string_view content)
{
    std::string sessionIdValue;
    std::string sourcePathValue;
    std::uint64_t totalLines = 0U;
    std::string analysisFormatValue = "plain";
    std::uint64_t infoLines = 0U;
    std::uint64_t warnLines = 0U;
    std::uint64_t errorLines = 0U;
    std::uint64_t otherLines = 0U;
    std::uint64_t blankLines = 0U;
    std::uint64_t minLines = 0U;
    std::string maxLinesValue;
    std::uint64_t minErrors = 0U;
    std::uint64_t minWarnings = 0U;
    std::string searchQuery;
    std::string contentSearch;
    std::string timeFromValue;
    std::string timeToValue;
    std::string lineLevelValue;
    std::string messageContains;
    std::string jsonKey;
    std::string searchQueryExpression;
    std::string searchHistoryValue;
    std::string reportFormatValue = "text";
    std::string reportSectionsValue = "all";
    std::string configPathValue;

    const std::vector<std::string> lines = foundation::split(content, '\n');

    for (const std::string& line : lines)
    {
        const std::string trimmed = foundation::trim(line);

        if (trimmed.empty() || trimmed.front() == '#')
        {
            continue;
        }

        const std::size_t separator = trimmed.find('=');

        if (separator == std::string::npos)
        {
            continue;
        }

        const std::string key = foundation::trim(trimmed.substr(0, separator));
        const std::string value = trimmed.substr(separator + 1);

        if (key == "session.id")
        {
            sessionIdValue = value;
        }
        else if (key == "source.path")
        {
            sourcePathValue = value;
        }
        else if (key == "analysis.totalLines")
        {
            if (!assignUint64Field(value, totalLines))
            {
                return foundation::Result<InvestigationSession>(foundation::Error(
                    foundation::ErrorCode::ParseError, "Invalid analysis.totalLines value."));
            }
        }
        else if (key == "analysis.format")
        {
            analysisFormatValue = value;
        }
        else if (key == "analysis.infoLines")
        {
            if (!assignUint64Field(value, infoLines))
            {
                return foundation::Result<InvestigationSession>(foundation::Error(
                    foundation::ErrorCode::ParseError, "Invalid analysis.infoLines value."));
            }
        }
        else if (key == "analysis.warningLines")
        {
            if (!assignUint64Field(value, warnLines))
            {
                return foundation::Result<InvestigationSession>(foundation::Error(
                    foundation::ErrorCode::ParseError, "Invalid analysis.warningLines value."));
            }
        }
        else if (key == "analysis.errorLines")
        {
            if (!assignUint64Field(value, errorLines))
            {
                return foundation::Result<InvestigationSession>(foundation::Error(
                    foundation::ErrorCode::ParseError, "Invalid analysis.errorLines value."));
            }
        }
        else if (key == "analysis.otherLines")
        {
            if (!assignUint64Field(value, otherLines))
            {
                return foundation::Result<InvestigationSession>(foundation::Error(
                    foundation::ErrorCode::ParseError, "Invalid analysis.otherLines value."));
            }
        }
        else if (key == "analysis.blankLines")
        {
            if (!assignUint64Field(value, blankLines))
            {
                return foundation::Result<InvestigationSession>(foundation::Error(
                    foundation::ErrorCode::ParseError, "Invalid analysis.blankLines value."));
            }
        }
        else if (key == "filter.minLines")
        {
            if (!assignUint64Field(value, minLines))
            {
                return foundation::Result<InvestigationSession>(foundation::Error(
                    foundation::ErrorCode::ParseError, "Invalid filter.minLines value."));
            }
        }
        else if (key == "filter.maxLines")
        {
            maxLinesValue = value;
        }
        else if (key == "filter.minErrors")
        {
            if (!assignUint64Field(value, minErrors))
            {
                return foundation::Result<InvestigationSession>(foundation::Error(
                    foundation::ErrorCode::ParseError, "Invalid filter.minErrors value."));
            }
        }
        else if (key == "filter.minWarnings")
        {
            if (!assignUint64Field(value, minWarnings))
            {
                return foundation::Result<InvestigationSession>(foundation::Error(
                    foundation::ErrorCode::ParseError, "Invalid filter.minWarnings value."));
            }
        }
        else if (key == "filter.searchQuery")
        {
            searchQuery = value;
        }
        else if (key == "filter.contentSearch")
        {
            contentSearch = value;
        }
        else if (key == "filter.timeFrom")
        {
            timeFromValue = value;
        }
        else if (key == "filter.timeTo")
        {
            timeToValue = value;
        }
        else if (key == "filter.lineLevel")
        {
            lineLevelValue = value;
        }
        else if (key == "filter.messageContains")
        {
            messageContains = value;
        }
        else if (key == "filter.jsonKey")
        {
            jsonKey = value;
        }
        else if (key == "search.query")
        {
            searchQueryExpression = value;
        }
        else if (key == "search.history")
        {
            searchHistoryValue = value;
        }
        else if (key == "report.format")
        {
            reportFormatValue = value;
        }
        else if (key == "report.sections")
        {
            reportSectionsValue = value;
        }
        else if (key == "config.path")
        {
            configPathValue = value;
        }
    }

    if (sourcePathValue.empty())
    {
        return foundation::Result<InvestigationSession>(
            foundation::Error(foundation::ErrorCode::ParseError, "Session file is missing source.path."));
    }

    const auto sessionIdResult = foundation::Uuid::parse(sessionIdValue);
    foundation::Uuid sessionId;

    if (sessionIdResult)
    {
        sessionId = *sessionIdResult;
    }

    const analysis::LogLevelCounts levelCounts =
        analysis::LogLevelCounts::fromCounts(infoLines, warnLines, errorLines, otherLines, blankLines);

    analysis::LogFormat analysisFormat = analysis::LogFormat::PlainText;
    const auto parsedAnalysisFormat = analysis::parseLogFormat(analysisFormatValue);

    if (parsedAnalysisFormat && *parsedAnalysisFormat != analysis::LogFormat::Auto &&
        *parsedAnalysisFormat != analysis::LogFormat::Unknown)
    {
        analysisFormat = *parsedAnalysisFormat;
    }

    const analysis::AnalysisModel model(foundation::Path(sourcePathValue), totalLines, levelCounts, analysisFormat);

    investigation::LineCountFilter lineFilter = lineFilterFromValues(minLines, maxLinesValue);

    investigation::LogLevelFilter levelFilter =
        investigation::LogLevelFilter::any().withMinimumErrors(minErrors).withMinimumWarnings(minWarnings);

    reporting::ReportOptions reportOptions;
    const auto parsedFormat = reporting::parseReportFormat(reportFormatValue);

    if (parsedFormat)
    {
        reportOptions.format = *parsedFormat;
    }

    reportOptions.sections = sectionsFromString(reportSectionsValue);

    investigation::InvestigationCriteria contentCriteria = contentCriteriaFromValues(
        contentSearch, timeFromValue, timeToValue, lineLevelValue, messageContains, jsonKey);

    if (!foundation::isBlank(searchQueryExpression))
    {
        const auto parsedQuery = search::parseSearchQuery(searchQueryExpression);

        if (parsedQuery)
        {
            contentCriteria.searchQuery = *parsedQuery;
        }
        else
        {
            contentCriteria.booleanQuery = searchQueryExpression;
        }
    }

    const search::SearchHistory searchHistory = search::SearchHistory::deserialize(searchHistoryValue);

    return foundation::Result<InvestigationSession>(InvestigationSession(
        sessionId, foundation::Path(sourcePathValue), model, lineFilter, levelFilter, searchQuery, contentCriteria,
        searchHistory, reportOptions, foundation::Path(configPathValue)));
}

} // namespace scope::workspace
