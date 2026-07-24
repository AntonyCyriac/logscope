/**
 * @file frequency_analyzer.cpp
 * @brief FrequencyAnalyzer implementation.
 */

#include "frequency_analyzer.hpp"

#include <algorithm>
#include <unordered_map>

namespace scope::analytics
{

namespace
{

std::vector<FrequencyEntry> topEntries(std::unordered_map<std::string, std::uint64_t> counts,
                                       const std::size_t limit)
{
    std::vector<FrequencyEntry> entries;
    entries.reserve(counts.size());

    for (auto& entry : counts)
    {
        entries.push_back(FrequencyEntry{std::move(entry.first), entry.second});
    }

    std::sort(entries.begin(),
              entries.end(),
              [](const FrequencyEntry& left, const FrequencyEntry& right) {
                  if (left.count != right.count)
                  {
                      return left.count > right.count;
                  }

                  return left.key < right.key;
              });

    if (entries.size() > limit)
    {
        entries.resize(limit);
    }

    return entries;
}

} // namespace

FrequencyResult FrequencyAnalyzer::analyze(const analysis::AnalysisModel& model,
                                           const AnalyticsConfig& config) const
{
    FrequencyResult result;
    result.setLevelCounts(model.levelCounts());

    if (!model.lineIndex().has_value())
    {
        return result;
    }

    std::unordered_map<std::string, std::uint64_t> messageCounts;
    std::unordered_map<std::string, std::uint64_t> correlationCounts;

    for (const analysis::IndexedLine& line : model.lineIndex()->lines())
    {
        if (!line.messageExcerpt.empty())
        {
            ++messageCounts[line.messageExcerpt];
        }

        if (!line.correlationId.empty())
        {
            ++correlationCounts[line.correlationId];
        }
    }

    result.setTopMessages(topEntries(std::move(messageCounts), config.topN));
    result.setTopCorrelationIds(topEntries(std::move(correlationCounts), config.topN));

    return result;
}

} // namespace scope::analytics
