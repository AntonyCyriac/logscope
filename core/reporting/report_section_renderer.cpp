/**
 * @file report_section_renderer.cpp
 * @brief Section registry implementation.
 */

#include "report_section_renderer.hpp"

namespace scope::reporting
{

namespace
{

class ContributorRenderer final : public ReportSectionRenderer
{
  public:
    explicit ContributorRenderer(std::unique_ptr<ReportSectionContributor> contributor)
        : m_contributor(std::move(contributor))
    {
    }

    [[nodiscard]] ReportSection section() const noexcept override
    {
        return m_contributor->section();
    }

    [[nodiscard]] ReportFragment render(const analysis::AnalysisModel& model,
                                        const ReportOptions& /*options*/) const override
    {
        return m_contributor->render(model);
    }

  private:
    std::unique_ptr<ReportSectionContributor> m_contributor;
};

constexpr ReportSection kSectionOrder[] = {ReportSection::ExecutiveSummary,
                                           ReportSection::Summary,
                                           ReportSection::LevelBreakdown,
                                           ReportSection::ErrorSummary,
                                           ReportSection::AnalyticsSummary,
                                           ReportSection::Timeline,
                                           ReportSection::Clusters,
                                           ReportSection::Charts,
                                           ReportSection::SourceMetadata,
                                           ReportSection::FormatsFooter};

} // namespace

ReportSectionRegistry::ReportSectionRegistry()
{
    registerBuiltInSectionRenderers(*this);
    registerAnalyticsSectionRenderers(*this);
    registerReportingContributors(*this);
    m_builtInRendererCount = m_renderers.size();
}

ReportSectionRegistry& ReportSectionRegistry::instance()
{
    static ReportSectionRegistry registry;

    return registry;
}

void ReportSectionRegistry::registerRenderer(std::unique_ptr<ReportSectionRenderer> renderer)
{
    m_renderers.push_back(std::move(renderer));
}

void ReportSectionRegistry::registerContributor(std::unique_ptr<ReportSectionContributor> contributor)
{
    registerRenderer(std::make_unique<ContributorRenderer>(std::move(contributor)));
}

void ReportSectionRegistry::clearPluginContributors()
{
    if (m_renderers.size() > m_builtInRendererCount)
    {
        m_renderers.resize(m_builtInRendererCount);
    }
}

const ReportSectionRenderer* ReportSectionRegistry::findRenderer(const ReportSection section) const
{
    for (const std::unique_ptr<ReportSectionRenderer>& renderer : m_renderers)
    {
        if (renderer->section() == section)
        {
            return renderer.get();
        }
    }

    return nullptr;
}

std::vector<ReportFragment> ReportSectionRegistry::renderFragments(const analysis::AnalysisModel& model,
                                                                     const ReportOptions& options) const
{
    std::vector<ReportFragment> fragments;

    for (const ReportSection section : kSectionOrder)
    {
        if (!options.sections.includes(section))
        {
            continue;
        }

        for (const std::unique_ptr<ReportSectionRenderer>& renderer : m_renderers)
        {
            if (renderer->section() != section)
            {
                continue;
            }

            ReportFragment fragment = renderer->render(model, options);

            if (!fragment.empty())
            {
                fragments.push_back(std::move(fragment));
            }
        }
    }

    return fragments;
}

} // namespace scope::reporting
