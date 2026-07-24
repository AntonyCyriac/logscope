/**
 * @file report_contributors.cpp
 * @brief Built-in report section contributors from extensions.
 */

#include "report_section_renderer.hpp"

#include <sstream>

#include "report_format.hpp"
#include "report_string_utils.hpp"

namespace scope::reporting
{

namespace
{

class FormatsFooterContributor final : public ReportSectionContributor
{
  public:
    [[nodiscard]] std::string id() const override
    {
        return "reporting.multi-format";
    }

    [[nodiscard]] ReportSection section() const noexcept override
    {
        return ReportSection::FormatsFooter;
    }

    [[nodiscard]] ReportFragment render(const analysis::AnalysisModel& /*model*/) const override
    {
        ReportFragment fragment;
        fragment.jsonKey = "formatsFooter";

        const std::string formats = "text, json, csv, markdown, html, pdf";

        fragment.textBody = std::string("--- Formats Available ---\n") + formats + '\n';
        fragment.markdownBody = "## Formats Available\n\n" + formats + "\n";
        fragment.jsonBody = "\n    \"formats\": " + escapeJsonString(formats);
        fragment.csvRows.push_back(CsvRow{"formatsFooter", "formats", formats});

        std::ostringstream html;
        html << "<section class=\"report-section\" id=\"formats-footer\">\n";
        html << "  <h2>Formats Available</h2>\n";
        html << "  <p>" << escapeHtml(formats) << "</p>\n";
        html << "</section>\n";
        fragment.htmlBody = html.str();

        return fragment;
    }
};

} // namespace

void registerReportingContributors(ReportSectionRegistry& registry)
{
    static bool registered = false;

    if (registered)
    {
        return;
    }

    registered = true;
    registry.registerContributor(std::make_unique<FormatsFooterContributor>());
}

} // namespace scope::reporting
