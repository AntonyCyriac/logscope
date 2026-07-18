/**
 * @file report_config.cpp
 * @brief Report configuration helpers.
 */

#include "report_config.hpp"

#include "output_format.hpp"

namespace scope::cli
{

reporting::ReportOptions buildReportOptions(const AnalyzeOptions& options,
                                            const configuration::ConfigurationManager& configurationManager)
{
    reporting::ReportOptions reportOptions;
    reportOptions.format = toReportFormat(options.format);

    if (options.sections)
    {
        reportOptions.sections = *options.sections;

        return reportOptions;
    }

    const auto& configuration = configurationManager.configuration();

    if (configuration.has("report.sections"))
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

    return reportOptions;
}

} // namespace scope::cli
