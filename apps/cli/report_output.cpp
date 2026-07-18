/**
 * @file report_output.cpp
 * @brief CLI report formatting implementation.
 */

#include "report_output.hpp"

#include "reporting.hpp"

namespace scope::cli
{

std::string formatAnalysisOutput(const analysis::AnalysisModel& model, const reporting::ReportOptions& options)
{
    const reporting::Report report = reporting::ReportGenerator{}.generate(model, options);

    return report.text();
}

} // namespace scope::cli
