/**
 * @file investigation_criteria.cpp
 * @brief InvestigationCriteria implementation.
 */

#include "investigation_criteria.hpp"

#include "foundation/string.hpp"
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
    if (filterQuery.has_value())
    {
        return foundation::Result<query::QueryNode>(*filterQuery);
    }

    if (!foundation::isBlank(filterExpression))
    {
        return query::parseFilterQuery(filterExpression);
    }

    return foundation::Result<query::QueryNode>(query::QueryNode::matchAll());
}

} // namespace scope::investigation
