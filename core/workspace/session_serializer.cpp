/**
 * @file session_serializer.cpp
 * @brief SessionSerializer implementation.
 */

#include "session_serializer.hpp"

#include <fstream>
#include <sstream>
#include "foundation/string.hpp"
#include "report_format.hpp"
#include "report_section.hpp"

namespace scope::workspace
{

namespace
{

constexpr std::string_view sessionVersion = "1.0";

std::string sectionsToString(const reporting::ReportSections& sections)
{
    if (sections.includes(reporting::ReportSection::Summary) &&
        sections.includes(reporting::ReportSection::LevelBreakdown) &&
        sections.includes(reporting::ReportSection::SourceMetadata))
    {
        return "all";
    }

    std::ostringstream output;
    bool wroteSection = false;

    if (sections.includes(reporting::ReportSection::Summary))
    {
        output << "summary";
        wroteSection = true;
    }

    if (sections.includes(reporting::ReportSection::LevelBreakdown))
    {
        if (wroteSection)
        {
            output << ',';
        }

        output << "levels";
        wroteSection = true;
    }

    if (sections.includes(reporting::ReportSection::SourceMetadata))
    {
        if (wroteSection)
        {
            output << ',';
        }

        output << "metadata";
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
            totalLines = std::stoull(value);
        }
        else if (key == "analysis.infoLines")
        {
            infoLines = std::stoull(value);
        }
        else if (key == "analysis.warningLines")
        {
            warnLines = std::stoull(value);
        }
        else if (key == "analysis.errorLines")
        {
            errorLines = std::stoull(value);
        }
        else if (key == "analysis.otherLines")
        {
            otherLines = std::stoull(value);
        }
        else if (key == "analysis.blankLines")
        {
            blankLines = std::stoull(value);
        }
        else if (key == "filter.minLines")
        {
            minLines = std::stoull(value);
        }
        else if (key == "filter.maxLines")
        {
            maxLinesValue = value;
        }
        else if (key == "filter.minErrors")
        {
            minErrors = std::stoull(value);
        }
        else if (key == "filter.minWarnings")
        {
            minWarnings = std::stoull(value);
        }
        else if (key == "filter.searchQuery")
        {
            searchQuery = value;
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

    const analysis::AnalysisModel model(foundation::Path(sourcePathValue), totalLines, levelCounts);

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

    return foundation::Result<InvestigationSession>(InvestigationSession(
        sessionId, foundation::Path(sourcePathValue), model, lineFilter, levelFilter, searchQuery, reportOptions,
        foundation::Path(configPathValue)));
}

} // namespace scope::workspace
