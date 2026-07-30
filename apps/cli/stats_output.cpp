/**
 * @file stats_output.cpp
 */

#include "stats_output.hpp"

#include <iomanip>
#include <ostream>

#include "foundation/process_memory.hpp"

namespace scope::cli
{

namespace
{

void printDurationMs(std::ostream& output, const scope::foundation::Duration& duration)
{
    const double milliseconds =
        static_cast<double>(duration.totalNanoseconds()) / static_cast<double>(1'000'000);

    output << std::fixed << std::setprecision(3) << milliseconds << " ms";
}

} // namespace

void printRunStats(const scope::analysis::AnalysisStats& analysisStats,
                   const scope::plugin::PluginLoadStats& pluginStats,
                   std::ostream& output)
{
    output << "========== RUN STATS ==========\n";
    output << "Parse duration : ";
    printDurationMs(output, analysisStats.parseDuration);
    output << '\n';
    output << "Line count     : " << analysisStats.lineCount << '\n';
    output << "Byte count     : " << analysisStats.byteCount << '\n';
    output << "Index reused   : " << (analysisStats.indexReused ? "yes" : "no") << '\n';

    if (pluginStats.attempted)
    {
        output << "Plugins loaded : " << pluginStats.loaded << '\n';
        output << "Plugins failed : " << pluginStats.failed << '\n';
        output << "Plugin paths skipped : " << pluginStats.skippedPaths << '\n';
        output << "Plugin load    : ";
        printDurationMs(output, pluginStats.loadDuration);
        output << '\n';
    }

    const auto memoryUsage = scope::foundation::currentProcessMemoryUsage();

    if (memoryUsage.has_value())
    {
        output << "Memory RSS     : " << memoryUsage->residentBytes << " bytes\n";
    }
    else
    {
        output << "Memory RSS     : unavailable\n";
    }

    output << "===============================\n";
}

} // namespace scope::cli
