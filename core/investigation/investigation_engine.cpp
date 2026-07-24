/**
 * @file investigation_engine.cpp
 * @brief InvestigationEngine implementation.
 */

#include "investigation_engine.hpp"

#include <unordered_map>

#include "correlation_analyzer.hpp"
#include "indexed_line_access.hpp"
#include "index_reader.hpp"
#include "log_macros.hpp"
#include "foundation/string.hpp"
#include "query_evaluator.hpp"
#include "query_planner.hpp"
#include "search_engine.hpp"

namespace scope::investigation
{

namespace
{

std::vector<analysis::IndexedLine> indexedLines(const analysis::AnalysisModel& model)
{
    if (!analysis::hasQueryableIndex(model))
    {
        return {};
    }

    return analysis::fetchIndexedLines(model);
}

std::vector<analysis::IndexedLine> candidateLines(const analysis::AnalysisModel& model,
                                                  const query::QueryNode& filterNode)
{
    if (model.indexStore() != nullptr)
    {
        if (const auto plan = storage::planQueryPushdown(filterNode))
        {
            const analysis::LineIndex* memoryIndex =
                model.lineIndex().has_value() ? &*model.lineIndex() : nullptr;
            const storage::IndexReader reader(memoryIndex, model.indexStore());
            const auto fetched = reader.linesMatchingWhere(plan->sqlWhere);

            if (fetched)
            {
                return *fetched;
            }
        }
    }

    return indexedLines(model);
}

CorrelationSummary toCorrelationSummary(const analytics::CorrelationResult& result)
{
    CorrelationSummary summary;

    for (const analytics::CorrelationEntry& entry : result.repeatedErrors())
    {
        CorrelationEntry converted;
        converted.key = entry.key;
        converted.count = entry.count;
        converted.lineNumbers = entry.lineNumbers;
        summary.addRepeatedError(std::move(converted));
    }

    for (const analytics::CorrelationEntry& entry : result.sharedCorrelationIds())
    {
        CorrelationEntry converted;
        converted.key = entry.key;
        converted.count = entry.count;
        converted.lineNumbers = entry.lineNumbers;
        summary.addSharedCorrelationId(std::move(converted));
    }

    return summary;
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

    if (!analysis::hasQueryableIndex(model))
    {
        return result;
    }

    result.indexedLineCount = analysis::indexedLineCountForModel(model);
    result.truncatedLineCount = analysis::truncatedLineCountForModel(model);

    const auto resolvedQuery = criteria.resolvedSearchQuery();
    search::SearchQuery activeQuery = search::SearchQuery::matchAll();

    if (resolvedQuery)
    {
        activeQuery = *resolvedQuery;
        result.searchQuerySummary = activeQuery.toString();
        result.searchMode = activeQuery.mode();
    }

    const search::SearchEngine searchEngine;
    query::QueryNode activeFilter = query::QueryNode::matchAll();
    const auto resolvedFilter = criteria.resolvedFilterQuery();

    if (resolvedFilter)
    {
        activeFilter = *resolvedFilter;
    }

    const query::QueryEvaluator filterEvaluator(activeFilter);
    const std::vector<analysis::IndexedLine> lines = candidateLines(model, activeFilter);

    for (const analysis::IndexedLine& line : lines)
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

        if (!filterEvaluator.matches(line))
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
    const analytics::CorrelationAnalyzer analyzer;

    return toCorrelationSummary(analyzer.analyze(model));
}

} // namespace scope::investigation
