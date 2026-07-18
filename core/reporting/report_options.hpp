/**
 * @file report_options.hpp
 * @brief Options controlling report generation.
 */

#pragma once

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

    /**
     * @brief Returns default options (text format, all sections).
     */
    [[nodiscard]] static ReportOptions defaults() noexcept;
};

} // namespace scope::reporting
