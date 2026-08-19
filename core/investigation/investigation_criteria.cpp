/**
 * @file investigation_criteria.cpp
 * @brief InvestigationCriteria implementation.
 */

#include "investigation_criteria.hpp"

#include "foundation/string.hpp"
#include "query_evaluator.hpp"
#include "query_parser.hpp"
#include "search_query_parser.hpp"

namespace scope::investigation
{

bool InvestigationCriteria::isActive() const noexcept
{
    return searchQuery.has_value() || !foundation::isBlank(booleanQuery) || !foundation::isBlank(contentSearch) ||
           !foundation::isBlank(filterExpression) || filterQuery.has_value() || timeRange.isActive() ||
           field.isActive();
}

foundation::Result<search::SearchQuery> InvestigationCriteria::resolvedSearchQuery() const noexcept
{
    if (searchQuery.has_value())
    {
        return foundation::Result<search::SearchQuery>(*searchQuery);
    }

    if (!foundation::isBlank(booleanQuery))
    {
        return search::parseSearchQuery(booleanQuery);
    }

    if (!foundation::isBlank(contentSearch))
    {
        if (searchMode == search::SearchMode::Regex)
        {
            return search::parseRegexQuery(contentSearch, caseSensitivity);
        }

        return foundation::Result<search::SearchQuery>(
            search::SearchQuery::term(contentSearch, search::SearchMode::Text, caseSensitivity));
    }

    return foundation::Result<search::SearchQuery>(search::SearchQuery::matchAll());
}

foundation::Result<query::QueryNode> InvestigationCriteria::resolvedFilterQuery() const noexcept
{
    foundation::Result<query::QueryNode> resolved(query::QueryNode::matchAll());

    if (filterQuery.has_value())
    {
        resolved = foundation::Result<query::QueryNode>(*filterQuery);
    }
    else if (!foundation::isBlank(filterExpression))
    {
        resolved = query::parseFilterQuery(filterExpression);
    }

    if (!resolved)
    {
        return resolved;
    }

    const auto validated = query::validateFilterSemantics(*resolved);

    if (!validated)
    {
        return foundation::Result<query::QueryNode>(validated.error());
    }

    return resolved;
}

void applyInvestigationConfiguration(InvestigationCriteria& criteria,
                                     const runtime::Configuration& configuration) noexcept
{
    if (configuration.has("investigation.search_provider"))
    {
        const auto providerResult = configuration.get("investigation.search_provider");

        if (providerResult && !providerResult->empty())
        {
            criteria.searchProviderId = *providerResult;
        }
    }
}

} // namespace scope::investigation
