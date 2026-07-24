/**
 * @file report_config.cpp
 * @brief Report configuration helpers.
 */

#include "report_config.hpp"

#include "foundation/string.hpp"
#include "output_format.hpp"
#include "report_format.hpp"

namespace scope::cli
{

namespace
{

bool parseBoolean(const std::string& value)
{
    const std::string normalized = foundation::toLower(foundation::trim(value));

    return normalized == "true" || normalized == "1" || normalized == "yes" || normalized == "on";
}

} // namespace

reporting::ReportOptions buildReportOptions(const AnalyzeOptions& options,
                                            const configuration::ConfigurationManager& configurationManager)
{
    reporting::ReportOptions reportOptions;
    const auto& configuration = configurationManager.configuration();

    if (configuration.has("report.format"))
    {
        const auto formatValue = configuration.get("report.format");

        if (formatValue)
        {
            const auto parsedFormat = reporting::parseReportFormat(*formatValue);

            if (parsedFormat)
            {
                reportOptions.format = *parsedFormat;
            }
        }
    }

    if (options.format)
    {
        reportOptions.format = toReportFormat(*options.format);
    }

    if (options.sections)
    {
        reportOptions.sections = *options.sections;
    }
    else if (configuration.has("report.sections"))
    {
        const auto sectionsValue = configuration.get("report.sections");

        if (sectionsValue)
        {
            const auto parsedSections = reporting::ReportSections::parse(*sectionsValue);

            if (parsedSections)
            {
                reportOptions.sections = *parsedSections;
            }
        }
    }

    if (configuration.has("report.include_charts"))
    {
        const auto includeChartsValue = configuration.get("report.include_charts");

        if (includeChartsValue)
        {
            reportOptions.includeCharts = parseBoolean(*includeChartsValue);
        }
    }

    if (configuration.has("report.template"))
    {
        const auto templateValue = configuration.get("report.template");

        if (templateValue)
        {
            reportOptions.reportTemplate = *templateValue;
        }
    }

    return reportOptions;
}

} // namespace scope::cli
