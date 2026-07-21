/**
 * @file investigation_engine.cpp
 * @brief InvestigationEngine implementation.
 */

#include "investigation_engine.hpp"

#include <unordered_map>

#include "log_macros.hpp"
#include "foundation/string.hpp"
#include "search_engine.hpp"

namespace scope::investigation
{

namespace
{

std::vector<analysis::IndexedLine> indexedLines(const analysis::AnalysisModel& model)
{
    if (!model.lineIndex().has_value())
    {
        return {};
    }

    return model.lineIndex()->lines();
}

} // namespace

InvestigationView InvestigationEngine::inspect(const analysis::AnalysisModel& model) const
{
    SCOPE_LOG_INFO("investigation", "Inspecting analysis for " + model.sourcePath().string());

    return InvestigationView(model);
}

bool InvestigationEngine::matches(const analysis::AnalysisModel& model,
                                  const LineCountFilter& filter) const noexcept
{
    return filter.matches(model);
}

bool InvestigationEngine::matches(const analysis::AnalysisModel& model,
                                  const LogLevelFilter& filter) const noexcept
{
    return filter.matches(model);
}

bool InvestigationEngine::searchSource(const analysis::AnalysisModel& model,
                                       const std::string_view query) const noexcept
{
    if (foundation::isBlank(query))
    {
        return true;
    }

    const std::string sourcePath = foundation::toLower(model.sourcePath().string());
    const std::string loweredQuery = foundation::toLower(query);

    return sourcePath.find(loweredQuery) != std::string::npos;
}

std::vector<analysis::IndexedLine> InvestigationEngine::searchContent(const analysis::AnalysisModel& model,
                                                                      const std::string_view query) const
{
    InvestigationCriteria criteria;
    criteria.contentSearch = std::string(query);

    const auto result = investigate(model, criteria);

    return result.matchingLines;
}

InvestigationResult InvestigationEngine::investigate(const analysis::AnalysisModel& model,
                                                     const InvestigationCriteria& criteria) const
{
    InvestigationResult result;

    if (!model.lineIndex().has_value())
    {
        return result;
    }

    const analysis::LineIndex& lineIndex = *model.lineIndex();
    result.indexedLineCount = lineIndex.indexedLineCount();
    result.truncatedLineCount = lineIndex.truncatedLineCount();

    const auto resolvedQuery = criteria.resolvedSearchQuery();
    search::SearchQuery activeQuery = search::SearchQuery::matchAll();

    if (resolvedQuery)
    {
        activeQuery = *resolvedQuery;
        result.searchQuerySummary = activeQuery.toString();
        result.searchMode = activeQuery.mode();
    }

    const search::SearchEngine searchEngine;

    for (const analysis::IndexedLine& line : lineIndex.lines())
    {
        if (!searchEngine.matches(line, activeQuery))
        {
            continue;
        }

        if (!criteria.timeRange.matches(line))
        {
            continue;
        }

        if (!criteria.field.matches(line))
        {
            continue;
        }

        result.matchingLines.push_back(line);
    }

    result.correlations = findCorrelations(model);

    return result;
}

CorrelationSummary InvestigationEngine::findCorrelations(const analysis::AnalysisModel& model) const
{
    CorrelationSummary summary;

    std::unordered_map<std::string, CorrelationEntry> repeatedErrors;
    std::unordered_map<std::string, CorrelationEntry> sharedIds;

    for (const analysis::IndexedLine& line : indexedLines(model))
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

    for (auto& entry : repeatedErrors)
    {
        if (entry.second.count > 1U)
        {
            summary.addRepeatedError(std::move(entry.second));
        }
    }

    for (auto& entry : sharedIds)
    {
        if (entry.second.count > 1U)
        {
            summary.addSharedCorrelationId(std::move(entry.second));
        }
    }

    return summary;
}

} // namespace scope::investigation
