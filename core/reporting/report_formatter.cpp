/**
 * @file report_formatter.cpp
 * @brief ReportFormatter implementation.
 */

#include "report_formatter.hpp"

#include "format_renderer.hpp"
#include "log_macros.hpp"
#include "report_section_renderer.hpp"

namespace scope::reporting
{

Report ReportFormatter::format(const analysis::AnalysisModel& model, const ReportOptions& options)
{
    SCOPE_LOG_INFO("reporting", "Formatting report for " + model.sourcePath().string());

    const std::vector<ReportFragment> fragments =
        ReportSectionRegistry::instance().renderFragments(model, options);

    return FormatRenderer::render(model, fragments, options);
}

} // namespace scope::reporting
