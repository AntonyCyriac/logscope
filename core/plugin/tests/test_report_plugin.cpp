/**
 * @file test_report_plugin.cpp
 * @brief In-tree test plugin for plugin loader unit tests (M12).
 */

#include <logscope/plugin/plugin.h>

extern "C" int logscope_plugin_register(const LogScopeHostApi* host)
{
    if (host == nullptr)
    {
        return 1;
    }

    const LogScopePluginInfo info{LOGSCOPE_PLUGIN_API_VERSION, "test.report", "1.0.0",
                                "Test plugin for unit tests."};

    return host->register_extension(host->context, &info);
}
