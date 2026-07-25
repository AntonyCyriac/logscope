/**
 * @file nl_query_translator.cpp
 */

#include "nl_query_translator.hpp"

#include "foundation/error.hpp"
#include "query_parser.hpp"

namespace scope::ai
{

NlQueryTranslator::NlQueryTranslator(const AiProvider& provider) noexcept : m_provider(provider) {}

foundation::Result<std::string> NlQueryTranslator::translateToFilterExpression(
    std::string_view naturalLanguageQuery) const
{
    if (naturalLanguageQuery.empty())
    {
        return foundation::Result<std::string>(foundation::Error(foundation::ErrorCode::InvalidArgument,
                                                                 "Natural language query must not be empty."));
    }

    const auto translated = m_provider.translateNlToFilter(naturalLanguageQuery);

    if (!translated)
    {
        return foundation::Result<std::string>(translated.error());
    }

    const auto parsed = query::parseFilterQuery(*translated);

    if (!parsed)
    {
        return foundation::Result<std::string>(foundation::Error(
            foundation::ErrorCode::ParseError,
            "AI provider returned invalid filter DSL: " + parsed.error().message()));
    }

    return foundation::Result<std::string>(*translated);
}

foundation::Result<query::QueryNode> NlQueryTranslator::translateToFilterQuery(
    std::string_view naturalLanguageQuery) const
{
    const auto expression = translateToFilterExpression(naturalLanguageQuery);

    if (!expression)
    {
        return foundation::Result<query::QueryNode>(expression.error());
    }

    return query::parseFilterQuery(*expression);
}

} // namespace scope::ai
