/**
 * @file analytics_output.cpp
 * @brief Analytics output formatting.
 */

#include "analytics_output.hpp"

#include <sstream>

namespace scope::cli
{

void writeAnalyticsOutput(const analytics::AnalyticsResult& result,
                          const OutputFormat format,
                          std::ostream& output)
{
    if (format == OutputFormat::Json)
    {
        output << "{\n";
        output << "  \"indexedLineCount\": " << result.indexedLineCount() << ",\n";
        output << "  \"truncatedLineCount\": " << result.truncatedLineCount() << ",\n";
        output << "  \"trendVerdict\": \"" << result.trends().verdict() << "\",\n";
        output << "  \"clusterCount\": " << result.clusters().clusters().size() << ",\n";
        output << "  \"repeatedErrorCount\": " << result.correlations().repeatedErrors().size() << "\n";
        output << "}\n";

        return;
    }

    output << "Analytics summary\n";
    output << "Indexed lines     : " << result.indexedLineCount() << '\n';

    if (result.truncatedLineCount() > 0U)
    {
        output << "Truncated lines   : " << result.truncatedLineCount() << '\n';
    }

    output << "Trend verdict     : " << result.trends().verdict() << '\n';
    output << "Error clusters    : " << result.clusters().clusters().size() << '\n';
    output << "Repeated errors   : " << result.correlations().repeatedErrors().size() << '\n';

    if (!result.frequency().topMessages().empty())
    {
        output << "\nTop messages:\n";

        for (const analytics::FrequencyEntry& entry : result.frequency().topMessages())
        {
            output << "  " << entry.key << " (" << entry.count << ")\n";
        }
    }

    if (!result.clusters().clusters().empty())
    {
        output << "\nError clusters:\n";

        for (const analytics::ErrorCluster& cluster : result.clusters().clusters())
        {
            output << "  " << cluster.signature << " (" << cluster.count << ")\n";
        }
    }

    if (result.timeline().hasData())
    {
        output << "\nTimeline buckets  : " << result.timeline().buckets().size() << '\n';
        output << "Bucket size (s)   : " << result.timeline().bucketSeconds() << '\n';
    }
}

} // namespace scope::cli
