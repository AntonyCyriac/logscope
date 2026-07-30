/**
 * @file stats_output.hpp
 * @brief CLI run statistics output for --stats.
 */

#pragma once

#include <iosfwd>

#include "analysis_stats.hpp"
#include "plugin_load_stats.hpp"

namespace scope::cli
{

void printRunStats(const scope::analysis::AnalysisStats& analysisStats,
                   const scope::plugin::PluginLoadStats& pluginStats,
                   std::ostream& output);

} // namespace scope::cli
