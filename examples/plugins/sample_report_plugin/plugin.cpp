#include <logscope/plugin/plugin.h>

#include <cstddef>

namespace
{

struct ReportContributorState
{
};

struct ReportContributorBundle
{
    LogScopeReportContributor contributor;
    ReportContributorState state;
};

void destroyReportContributor(void* instance)
{
    auto* bundle = reinterpret_cast<ReportContributorBundle*>(reinterpret_cast<char*>(instance) -
                                                              offsetof(ReportContributorBundle, state));
    delete bundle;
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
    auto* bundle = new ReportContributorBundle();
    bundle->contributor.instance = &bundle->state;
    bundle->contributor.vtable = &kReportContributorVTable;

    return &bundle->contributor;
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
