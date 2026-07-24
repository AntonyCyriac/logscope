/**
 * @file report_analytics_sections.cpp
 * @brief M9 analytics report section renderers.
 */

#include "report_section_renderer.hpp"

#include <sstream>

#include "analytics_engine.hpp"
#include "ascii_chart_renderer.hpp"
#include "chart_model.hpp"
#include "report_string_utils.hpp"
#include "svg_chart_renderer.hpp"

namespace scope::reporting
{

namespace
{

analytics::AnalyticsResult buildAnalyticsResult(const analysis::AnalysisModel& model, const ReportOptions& options)
{
    analytics::AnalyticsConfig config = options.analyticsConfig;
    config.includeTimeline = options.includeTimeline;

    return analytics::AnalyticsEngine{}.analyze(model, config);
}

void appendCsvRow(std::vector<CsvRow>& rows,
                  const std::string_view section,
                  const std::string_view key,
                  const std::uint64_t value)
{
    rows.push_back(CsvRow{std::string(section), std::string(key), std::to_string(value)});
}

void appendCsvRow(std::vector<CsvRow>& rows,
                  const std::string_view section,
                  const std::string_view key,
                  const std::string_view value)
{
    rows.push_back(CsvRow{std::string(section), std::string(key), std::string(value)});
}

class AnalyticsSummaryRenderer final : public ReportSectionRenderer
{
  public:
    [[nodiscard]] ReportSection section() const noexcept override
    {
        return ReportSection::AnalyticsSummary;
    }

    [[nodiscard]] ReportFragment render(const analysis::AnalysisModel& model,
                                        const ReportOptions& options) const override
    {
        ReportFragment fragment;
        fragment.jsonKey = "analytics";

        const analytics::AnalyticsResult analytics = buildAnalyticsResult(model, options);

        {
            std::ostringstream text;
            text << "--- Analytics ---" << '\n';
            text << "Trend verdict     : " << analytics.trends().verdict() << '\n';
            text << "Error clusters    : " << analytics.clusters().clusters().size() << '\n';
            text << "Repeated errors   : " << analytics.correlations().repeatedErrors().size() << '\n';

            if (!analytics.frequency().topMessages().empty())
            {
                text << "Top message       : " << analytics.frequency().topMessages().front().key << " ("
                     << analytics.frequency().topMessages().front().count << ")\n";
            }

            fragment.textBody = text.str();
        }

        {
            std::ostringstream markdown;
            markdown << "## Analytics\n\n";
            markdown << "| Metric | Value |\n";
            markdown << "|--------|-------|\n";
            markdown << "| Trend verdict | " << analytics.trends().verdict() << " |\n";
            markdown << "| Error clusters | " << analytics.clusters().clusters().size() << " |\n";
            markdown << "| Repeated errors | " << analytics.correlations().repeatedErrors().size() << " |\n";
            fragment.markdownBody = markdown.str();
        }

        appendCsvRow(fragment.csvRows, "analytics", "trendVerdict", analytics.trends().verdict());
        appendCsvRow(fragment.csvRows, "analytics", "clusterCount", analytics.clusters().clusters().size());

        {
            std::ostringstream html;
            html << "<section class=\"report-section\" id=\"analytics\">\n";
            html << "  <h2>Analytics</h2>\n";
            html << "  <table><tbody>\n";
            html << "    <tr><th>Trend verdict</th><td>" << escapeHtml(analytics.trends().verdict()) << "</td></tr>\n";
            html << "    <tr><th>Error clusters</th><td>" << analytics.clusters().clusters().size()
                 << "</td></tr>\n";
            html << "  </tbody></table>\n";
            html << "</section>\n";
            fragment.htmlBody = html.str();
        }

        return fragment;
    }
};

class TimelineRenderer final : public ReportSectionRenderer
{
  public:
    [[nodiscard]] ReportSection section() const noexcept override
    {
        return ReportSection::Timeline;
    }

    [[nodiscard]] ReportFragment render(const analysis::AnalysisModel& model,
                                        const ReportOptions& options) const override
    {
        ReportFragment fragment;
        fragment.jsonKey = "timeline";

        const analytics::AnalyticsResult analytics = buildAnalyticsResult(model, options);

        if (!analytics.timeline().hasData())
        {
            return fragment;
        }

        const TimeSeriesChart chart = buildTimelineChart(analytics.timeline());
        const std::string asciiChart = renderAsciiTimeSeriesChart(chart);
        const std::string markdownChart = renderMarkdownTimeSeriesChart(chart);
        const std::string svgChart = renderSvgTimeSeriesChart(chart);

        {
            std::ostringstream text;
            text << "--- Timeline ---" << '\n';
            text << "Bucket size (s)   : " << analytics.timeline().bucketSeconds() << '\n';
            text << asciiChart;
            fragment.textBody = text.str();
        }

        fragment.markdownBody = std::string("## Timeline\n\n") + markdownChart;

        for (const TimeSeriesPoint& point : chart.points)
        {
            appendCsvRow(fragment.csvRows, "timeline", point.label + ".errors", point.errorCount);
        }

        {
            std::ostringstream html;
            html << "<section class=\"report-section\" id=\"timeline\">\n";
            html << "  <h2>Timeline</h2>\n";
            html << "  <div class=\"chart\">" << svgChart << "</div>\n";
            html << "</section>\n";
            fragment.htmlBody = html.str();
        }

        return fragment;
    }
};

class ClustersRenderer final : public ReportSectionRenderer
{
  public:
    [[nodiscard]] ReportSection section() const noexcept override
    {
        return ReportSection::Clusters;
    }

    [[nodiscard]] ReportFragment render(const analysis::AnalysisModel& model,
                                        const ReportOptions& options) const override
    {
        ReportFragment fragment;
        fragment.jsonKey = "clusters";

        const analytics::AnalyticsResult analytics = buildAnalyticsResult(model, options);

        {
            std::ostringstream text;
            text << "--- Error Clusters ---" << '\n';

            for (const analytics::ErrorCluster& cluster : analytics.clusters().clusters())
            {
                text << cluster.signature << " (" << cluster.count << ")\n";
                text << "  sample: " << cluster.sampleMessage << '\n';
            }

            fragment.textBody = text.str();
        }

        {
            std::ostringstream markdown;
            markdown << "## Error Clusters\n\n";

            for (const analytics::ErrorCluster& cluster : analytics.clusters().clusters())
            {
                markdown << "- `" << cluster.signature << "` (" << cluster.count << ")\n";
            }

            fragment.markdownBody = markdown.str();
        }

        for (const analytics::ErrorCluster& cluster : analytics.clusters().clusters())
        {
            appendCsvRow(fragment.csvRows, "clusters", cluster.signature, cluster.count);
        }

        {
            std::ostringstream html;
            html << "<section class=\"report-section\" id=\"clusters\">\n";
            html << "  <h2>Error Clusters</h2>\n";
            html << "  <ul>\n";

            for (const analytics::ErrorCluster& cluster : analytics.clusters().clusters())
            {
                html << "    <li><code>" << escapeHtml(cluster.signature) << "</code> ("
                     << cluster.count << ")</li>\n";
            }

            html << "  </ul>\n";
            html << "</section>\n";
            fragment.htmlBody = html.str();
        }

        return fragment;
    }
};

void registerAnalyticsSectionRenderersImpl(ReportSectionRegistry& registry)
{
    registry.registerRenderer(std::make_unique<AnalyticsSummaryRenderer>());
    registry.registerRenderer(std::make_unique<TimelineRenderer>());
    registry.registerRenderer(std::make_unique<ClustersRenderer>());
}

} // namespace

void registerAnalyticsSectionRenderers(ReportSectionRegistry& registry)
{
    registerAnalyticsSectionRenderersImpl(registry);
}

} // namespace scope::reporting
