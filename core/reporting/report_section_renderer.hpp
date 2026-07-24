/**
 * @file report_section_renderer.hpp
 * @brief Section registry and built-in report section renderers.
 */

#pragma once

#include <memory>
#include <vector>

#include "analysis_model.hpp"
#include "report_fragment.hpp"
#include "report_options.hpp"
#include "report_section.hpp"
#include "report_section_contributor.hpp"

namespace scope::reporting
{

/**
 * @brief Renders one logical report section into a fragment.
 */
class ReportSectionRenderer
{
  public:
    virtual ~ReportSectionRenderer() = default;

    [[nodiscard]] virtual ReportSection section() const noexcept = 0;

    [[nodiscard]] virtual ReportFragment render(const analysis::AnalysisModel& model,
                                                const ReportOptions& options) const = 0;
};

/**
 * @brief Registry of built-in and contributed section renderers.
 */
class ReportSectionRegistry
{
  public:
    [[nodiscard]] static ReportSectionRegistry& instance();

    void registerRenderer(std::unique_ptr<ReportSectionRenderer> renderer);

    void registerContributor(std::unique_ptr<ReportSectionContributor> contributor);

    [[nodiscard]] std::vector<ReportFragment>
    renderFragments(const analysis::AnalysisModel& model, const ReportOptions& options) const;

  private:
    ReportSectionRegistry();

    [[nodiscard]] const ReportSectionRenderer* findRenderer(ReportSection section) const;

    std::vector<std::unique_ptr<ReportSectionRenderer>> m_renderers;
};

/**
 * @brief Registers built-in report section renderers.
 */
void registerBuiltInSectionRenderers(ReportSectionRegistry& registry);

/**
 * @brief Registers built-in report section contributors from extensions.
 */
void registerReportingContributors(ReportSectionRegistry& registry);

} // namespace scope::reporting
