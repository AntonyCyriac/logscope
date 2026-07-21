/**
 * @file search_query.cpp
 * @brief SearchQuery implementation.
 */

#include "search_query.hpp"

#include <sstream>

#include "foundation/string.hpp"

namespace scope::search
{

namespace
{

std::string quoteTerm(const std::string& value)
{
    if (value.find(' ') == std::string::npos && value.find('"') == std::string::npos)
    {
        return value;
    }

    std::ostringstream output;
    output << '"';

    for (const char character : value)
    {
        if (character == '"')
        {
            output << "\\\"";
        }
        else
        {
            output << character;
        }
    }

    output << '"';

    return output.str();
}

} // namespace

SearchQuery::SearchQuery() = default;

SearchQuery::SearchQuery(SearchQuery&&) noexcept = default;

SearchQuery& SearchQuery::operator=(SearchQuery&&) noexcept = default;

SearchQuery::SearchQuery(const SearchQuery& other)
    : m_kind(other.m_kind),
      m_mode(other.m_mode),
      m_caseSensitivity(other.m_caseSensitivity),
      m_term(other.m_term),
      m_left(other.m_left ? std::make_unique<SearchQuery>(*other.m_left) : nullptr),
      m_right(other.m_right ? std::make_unique<SearchQuery>(*other.m_right) : nullptr),
      m_operand(other.m_operand ? std::make_unique<SearchQuery>(*other.m_operand) : nullptr)
{
}

SearchQuery& SearchQuery::operator=(const SearchQuery& other)
{
    if (this != &other)
    {
        m_kind = other.m_kind;
        m_mode = other.m_mode;
        m_caseSensitivity = other.m_caseSensitivity;
        m_term = other.m_term;
        m_left = other.m_left ? std::make_unique<SearchQuery>(*other.m_left) : nullptr;
        m_right = other.m_right ? std::make_unique<SearchQuery>(*other.m_right) : nullptr;
        m_operand = other.m_operand ? std::make_unique<SearchQuery>(*other.m_operand) : nullptr;
    }

    return *this;
}

SearchQuery::~SearchQuery() = default;

SearchQuery::SearchQuery(const Kind kind) : m_kind(kind) {}

SearchQuery SearchQuery::matchAll() noexcept
{
    return SearchQuery(Kind::MatchAll);
}

SearchQuery SearchQuery::term(const std::string value, const SearchMode mode, const CaseSensitivity caseSensitivity)
{
    SearchQuery query(Kind::Term);
    query.m_term = std::move(value);
    query.m_mode = mode;
    query.m_caseSensitivity = caseSensitivity;

    return query;
}

SearchQuery SearchQuery::andQuery(SearchQuery left, SearchQuery right)
{
    SearchQuery query(Kind::And);
    query.m_left = std::make_unique<SearchQuery>(std::move(left));
    query.m_right = std::make_unique<SearchQuery>(std::move(right));

    return query;
}

SearchQuery SearchQuery::orQuery(SearchQuery left, SearchQuery right)
{
    SearchQuery query(Kind::Or);
    query.m_left = std::make_unique<SearchQuery>(std::move(left));
    query.m_right = std::make_unique<SearchQuery>(std::move(right));

    return query;
}

SearchQuery SearchQuery::notQuery(SearchQuery operand)
{
    SearchQuery query(Kind::Not);
    query.m_operand = std::make_unique<SearchQuery>(std::move(operand));

    return query;
}

SearchQuery::Kind SearchQuery::kind() const noexcept
{
    return m_kind;
}

SearchMode SearchQuery::mode() const noexcept
{
    return m_mode;
}

CaseSensitivity SearchQuery::caseSensitivity() const noexcept
{
    return m_caseSensitivity;
}

const std::string& SearchQuery::term() const noexcept
{
    return m_term;
}

const SearchQuery* SearchQuery::left() const noexcept
{
    return m_left.get();
}

const SearchQuery* SearchQuery::right() const noexcept
{
    return m_right.get();
}

const SearchQuery* SearchQuery::operand() const noexcept
{
    return m_operand.get();
}

bool SearchQuery::isActive() const noexcept
{
    if (m_kind == Kind::MatchAll)
    {
        return false;
    }

    if (m_kind == Kind::Term)
    {
        return !foundation::isBlank(m_term);
    }

    return true;
}

std::string SearchQuery::toString() const
{
    switch (m_kind)
    {
    case Kind::MatchAll:
        return {};
    case Kind::Term:
        return quoteTerm(m_term);
    case Kind::And:
        return '(' + m_left->toString() + " AND " + m_right->toString() + ')';
    case Kind::Or:
        return '(' + m_left->toString() + " OR " + m_right->toString() + ')';
    case Kind::Not:
        return "(NOT " + m_operand->toString() + ')';
    }

    return {};
}

} // namespace scope::search
