/**
 * @file format_renderer.hpp
 * @brief Assembles report fragments into final output formats.
 */

#pragma once

#include <vector>

#include "analysis_model.hpp"
#include "report.hpp"
#include "report_fragment.hpp"
#include "report_options.hpp"

namespace scope::reporting
{

/**
 * @brief Builds a final report from rendered section fragments.
 */
class FormatRenderer
{
  public:
    [[nodiscard]] static Report render(const analysis::AnalysisModel& model,
                                       const std::vector<ReportFragment>& fragments,
                                       const ReportOptions& options);
};

} // namespace scope::reporting
