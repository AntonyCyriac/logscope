#include <logscope/plugin/plugin.h>

namespace
{

struct ReportContributorState
{
};

void destroyReportContributor(void* instance)
{
    delete static_cast<ReportContributorState*>(instance);
}

int renderReportContributor(void* instance, uint64_t /*totalLines*/, LogScopeReportFragment* out)
{
    (void)instance;

    if (out == nullptr)
    {
        return 1;
    }

    out->text_body = "Sample plugin report section\n";
    out->html_body = "<section><h2>Sample Plugin</h2><p>Report contributor loaded.</p></section>";
    out->json_key = "sampleReport";
    out->json_body = "\"sample\"";

    return 0;
}

const LogScopeReportContributorVTable kReportContributorVTable{destroyReportContributor,
                                                                 renderReportContributor};

LogScopeReportContributor* createReportContributor()
{
    auto* contributor = new LogScopeReportContributor();
    contributor->instance = new ReportContributorState();
    contributor->vtable = &kReportContributorVTable;

    return contributor;
}

} // namespace

extern "C" int logscope_plugin_register(const LogScopeHostApi* host)
{
    if (host == nullptr)
    {
        return 1;
    }

    const LogScopePluginInfo info{LOGSCOPE_PLUGIN_API_VERSION, "sample.report", "1.0.0",
                                  "Sample report section contributor plugin."};

    if (host->register_extension(host->context, &info) != 0)
    {
        return 1;
    }

    return host->register_report_contributor(host->context, "sample.report", &createReportContributor);
}
