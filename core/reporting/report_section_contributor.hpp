/**
 * @file report_section_contributor.hpp
 * @brief Plugin hook for custom report sections (pre-M12 static registration).
 */

#pragma once

#include <memory>
#include <string>

#include "analysis_model.hpp"
#include "report_fragment.hpp"
#include "report_section.hpp"

namespace scope::reporting
{

/**
 * @brief Contributes an optional report section from a built-in extension.
 */
class ReportSectionContributor
{
  public:
    virtual ~ReportSectionContributor() = default;

    [[nodiscard]] virtual std::string id() const = 0;

    [[nodiscard]] virtual ReportSection section() const noexcept = 0;

    [[nodiscard]] virtual ReportFragment render(const analysis::AnalysisModel& model) const = 0;
};

} // namespace scope::reporting
