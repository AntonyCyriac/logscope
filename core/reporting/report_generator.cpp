/**
 * @file report_generator.cpp
 * @brief ReportGenerator implementation.
 */

#include "report_generator.hpp"

#include "log_macros.hpp"
#include "report_formatter.hpp"

namespace scope::reporting
{

Report ReportGenerator::generate(const analysis::AnalysisModel& model, const ReportOptions& options) const
{
    SCOPE_LOG_INFO("reporting", "Generating report for " + model.sourcePath().string());

    return ReportFormatter::format(model, options);
}

} // namespace scope::reporting
