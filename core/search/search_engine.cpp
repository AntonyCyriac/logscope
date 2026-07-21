/**
 * @file search_engine.cpp
 * @brief SearchEngine implementation.
 */

#include "search_engine.hpp"

#include <regex>

#include "text_matcher.hpp"

namespace scope::search
{

namespace
{

bool matchesTerm(const analysis::IndexedLine& line, const std::string& term, const SearchMode mode,
                 const CaseSensitivity caseSensitivity)
{
    if (mode == SearchMode::Regex)
    {
        if (term.empty() || term.size() > maxRegexPatternLength)
        {
            return false;
        }

        try
        {
            const std::regex pattern(
                term, caseSensitivity == CaseSensitivity::Sensitive ? std::regex::ECMAScript
                                                                    : std::regex::ECMAScript | std::regex::icase);

            return std::regex_search(line.contentExcerpt, pattern) ||
                   std::regex_search(line.messageExcerpt, pattern);
        }
        catch (const std::regex_error&)
        {
            return false;
        }
    }

    return textContains(line.contentExcerpt, term, caseSensitivity) ||
           textContains(line.messageExcerpt, term, caseSensitivity);
}

bool matchesQuery(const analysis::IndexedLine& line, const SearchQuery& query)
{
    switch (query.kind())
    {
    case SearchQuery::Kind::MatchAll:
        return true;
    case SearchQuery::Kind::Term:
        return matchesTerm(line, query.term(), query.mode(), query.caseSensitivity());
    case SearchQuery::Kind::And:
        return matchesQuery(line, *query.left()) && matchesQuery(line, *query.right());
    case SearchQuery::Kind::Or:
        return matchesQuery(line, *query.left()) || matchesQuery(line, *query.right());
    case SearchQuery::Kind::Not:
        return !matchesQuery(line, *query.operand());
    }

    return false;
}

} // namespace

bool SearchEngine::matches(const analysis::IndexedLine& line, const SearchQuery& query) const noexcept
{
    return matchesQuery(line, query);
}

std::vector<analysis::IndexedLine> SearchEngine::search(const analysis::LineIndex& index,
                                                        const SearchQuery& query) const
{
    return search(index.lines(), query);
}

std::vector<analysis::IndexedLine> SearchEngine::search(const std::vector<analysis::IndexedLine>& lines,
                                                        const SearchQuery& query) const
{
    std::vector<analysis::IndexedLine> matches;

    for (const analysis::IndexedLine& line : lines)
    {
        if (matchesQuery(line, query))
        {
            matches.push_back(line);
        }
    }

    return matches;
}

} // namespace scope::search
