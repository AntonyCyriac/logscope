/**
 * @file correlation_analyzer.cpp
 * @brief CorrelationAnalyzer implementation.
 */

#include "correlation_analyzer.hpp"

#include <unordered_map>

#include "log_line_classifier.hpp"
#include "indexed_line_access.hpp"

namespace scope::analytics
{

CorrelationResult CorrelationAnalyzer::analyze(const analysis::AnalysisModel& model) const
{
    CorrelationResult result;

    if (!analysis::hasQueryableIndex(model))
    {
        return result;
    }

    std::unordered_map<std::string, CorrelationEntry> repeatedErrors;
    std::unordered_map<std::string, CorrelationEntry> sharedIds;

    for (const analysis::IndexedLine& line : analysis::fetchIndexedLines(model))
    {
        if (line.level == analysis::DetectedLogLevel::Error && !line.messageExcerpt.empty())
        {
            CorrelationEntry& entry = repeatedErrors[line.messageExcerpt];
            entry.key = line.messageExcerpt;
            ++entry.count;
            entry.lineNumbers.push_back(line.lineNumber);
        }

        if (!line.correlationId.empty())
        {
            CorrelationEntry& entry = sharedIds[line.correlationId];
            entry.key = line.correlationId;
            ++entry.count;
            entry.lineNumbers.push_back(line.lineNumber);
        }
    }

    std::vector<CorrelationEntry> repeatedErrorEntries;
    repeatedErrorEntries.reserve(repeatedErrors.size());

    for (auto& entry : repeatedErrors)
    {
        if (entry.second.count > 1U)
        {
            repeatedErrorEntries.push_back(std::move(entry.second));
        }
    }

    std::vector<CorrelationEntry> sharedIdEntries;
    sharedIdEntries.reserve(sharedIds.size());

    for (auto& entry : sharedIds)
    {
        if (entry.second.count > 1U)
        {
            sharedIdEntries.push_back(std::move(entry.second));
        }
    }

    result.setRepeatedErrors(std::move(repeatedErrorEntries));
    result.setSharedCorrelationIds(std::move(sharedIdEntries));

    return result;
}

} // namespace scope::analytics
