/**
 * @file plugin.hpp
 * @brief C++ helpers for LogScope plugin authors (M12).
 */

#pragma once

#include <logscope/plugin/plugin.h>

#include <utility>

namespace logscope::plugin
{

inline int registerExtension(const LogScopeHostApi* host, const LogScopePluginInfo& info)
{
    if (host == nullptr || host->register_extension == nullptr)
    {
        return 1;
    }

    return host->register_extension(host->context, &info);
}

inline LogScopePluginInfo makeInfo(const char* id, const char* version, const char* description)
{
    return LogScopePluginInfo{LOGSCOPE_PLUGIN_API_VERSION, id, version, description};
}

} // namespace logscope::plugin
