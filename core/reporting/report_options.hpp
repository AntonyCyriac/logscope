/**
 * @file report_options.hpp
 * @brief Options controlling report generation.
 */

#pragma once

#include <string>

#include "analytics_config.hpp"
#include "report_format.hpp"
#include "report_section.hpp"

namespace scope::reporting
{

/**
 * @brief Controls report format and included sections.
 */
struct ReportOptions
{
    ReportFormat format = ReportFormat::Text;
    ReportSections sections = ReportSections::all();
    bool includeCharts = true;
    bool includeTimeline = true;
    std::string reportTemplate;
    analytics::AnalyticsConfig analyticsConfig;

    /**
     * @brief Returns default options (text format, all sections).
     */
    [[nodiscard]] static ReportOptions defaults() noexcept;
};

} // namespace scope::reporting
