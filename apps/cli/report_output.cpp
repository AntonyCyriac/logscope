/**
 * @file report_output.cpp
 * @brief CLI report formatting implementation.
 */

#include "report_output.hpp"

#include "reporting.hpp"

namespace scope::cli
{

reporting::Report generateAnalysisReport(const analysis::AnalysisModel& model,
                                         const reporting::ReportOptions& options)
{
    return reporting::ReportGenerator{}.generate(model, options);
}

} // namespace scope::cli
