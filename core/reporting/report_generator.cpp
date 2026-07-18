/**
 * @file report_generator.cpp
 * @brief ReportGenerator implementation.
 */

#include "report_generator.hpp"

#include <sstream>

#include "log_macros.hpp"

namespace scope::reporting
{

Report ReportGenerator::generate(const analysis::AnalysisModel& model) const
{
    SCOPE_LOG_INFO("reporting", "Generating report for " + model.sourcePath().string());

    std::ostringstream output;

    output << "========== LOGSCOPE REPORT ==========" << '\n';
    output << "Source          : " << model.sourcePath().string() << '\n';
    output << "Total log lines : " << model.totalLines() << '\n';
    output << "=====================================";

    return Report(output.str());
}

} // namespace scope::reporting
