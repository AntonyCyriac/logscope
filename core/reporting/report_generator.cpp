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

    const analysis::LogLevelCounts& levels = model.levelCounts();

    std::ostringstream output;

    output << "========== LOGSCOPE REPORT ==========" << '\n';
    output << "Source          : " << model.sourcePath().string() << '\n';
    output << "Total log lines : " << model.totalLines() << '\n';
    output << "Error lines     : " << levels.errorLines() << '\n';
    output << "Warning lines   : " << levels.warnLines() << '\n';
    output << "Info lines      : " << levels.infoLines() << '\n';
    output << "=====================================";

    return Report(output.str());
}

} // namespace scope::reporting
