#include <logscope/plugin/plugin.h>

namespace
{

struct SearchProviderState
{
};

void destroySearchProvider(void* instance)
{
    delete static_cast<SearchProviderState*>(instance);
}

int searchWithProvider(void* /*instance*/, const char* /*query*/, LogScopeReportFragment* /*out*/)
{
    return 0;
}

const LogScopeSearchProviderVTable kSearchProviderVTable{destroySearchProvider, searchWithProvider};

LogScopeSearchProvider* createSearchProvider()
{
    auto* provider = new LogScopeSearchProvider();
    provider->instance = new SearchProviderState();
    provider->vtable = &kSearchProviderVTable;

    return provider;
}

} // namespace

extern "C" int logscope_plugin_register(const LogScopeHostApi* host)
{
    if (host == nullptr)
    {
        return 1;
    }

    const LogScopePluginInfo info{LOGSCOPE_PLUGIN_API_VERSION, "sample.search", "1.0.0",
                                  "Sample substring search provider plugin."};

    if (host->register_extension(host->context, &info) != 0)
    {
        return 1;
    }

    return host->register_search_provider(host->context, "sample.search", &createSearchProvider);
}
