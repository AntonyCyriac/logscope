/**
 * @file search_query_parser.cpp
 * @brief Boolean search query parser implementation.
 */

#include "search_query_parser.hpp"

#include <cctype>
#include <regex>
#include <string>
#include <vector>

#include "foundation/error.hpp"
#include "foundation/string.hpp"
#include "search_engine.hpp"

namespace scope::search
{

namespace
{

enum class TokenKind
{
    End,
    Term,
    And,
    Or,
    Not,
    LeftParen,
    RightParen
};

struct Token
{
    TokenKind kind{TokenKind::End};
    std::string value;
};

class Parser
{
  public:
    explicit Parser(std::string_view input) : m_input(input) {}

    [[nodiscard]] foundation::Result<SearchQuery> parseExpression()
    {
        const auto tokensResult = tokenize();

        if (!tokensResult)
        {
            return foundation::Result<SearchQuery>(tokensResult.error());
        }

        m_tokens = *tokensResult;
        m_index = 0U;

        if (m_tokens.empty() || (m_tokens.size() == 1U && m_tokens[0].kind == TokenKind::End))
        {
            return foundation::Result<SearchQuery>(SearchQuery::matchAll());
        }

        auto expressionResult = parseOrExpression();

        if (!expressionResult)
        {
            return expressionResult;
        }

        if (current().kind != TokenKind::End)
        {
            return foundation::Result<SearchQuery>(foundation::Error(
                foundation::ErrorCode::InvalidArgument, "Unexpected token in search query."));
        }

        return expressionResult;
    }

  private:
    [[nodiscard]] foundation::Result<std::vector<Token>> tokenize()
    {
        std::vector<Token> tokens;
        std::size_t index = 0U;

        while (index < m_input.size())
        {
            const char character = m_input[index];

            if (std::isspace(static_cast<unsigned char>(character)) != 0)
            {
                ++index;
                continue;
            }

            if (character == '(')
            {
                tokens.push_back(Token{TokenKind::LeftParen, "("});
                ++index;
                continue;
            }

            if (character == ')')
            {
                tokens.push_back(Token{TokenKind::RightParen, ")"});
                ++index;
                continue;
            }

            if (character == '"')
            {
                ++index;
                std::string value;
                bool closed = false;

                while (index < m_input.size())
                {
                    const char currentCharacter = m_input[index];

                    if (currentCharacter == '\\' && index + 1U < m_input.size())
                    {
                        value.push_back(m_input[index + 1U]);
                        index += 2U;
                        continue;
                    }

                    if (currentCharacter == '"')
                    {
                        ++index;
                        closed = true;
                        break;
                    }

                    value.push_back(currentCharacter);
                    ++index;
                }

                if (!closed)
                {
                    return foundation::Result<std::vector<Token>>(foundation::Error(
                        foundation::ErrorCode::InvalidArgument, "Unterminated quoted term in search query."));
                }

                tokens.push_back(Token{TokenKind::Term, value});
                continue;
            }

            std::size_t end = index;

            while (end < m_input.size() && std::isspace(static_cast<unsigned char>(m_input[end])) == 0 &&
                   m_input[end] != '(' && m_input[end] != ')')
            {
                ++end;
            }

            const std::string word = foundation::toLower(std::string(m_input.substr(index, end - index)));

            if (word == "and")
            {
                tokens.push_back(Token{TokenKind::And, word});
            }
            else if (word == "or")
            {
                tokens.push_back(Token{TokenKind::Or, word});
            }
            else if (word == "not")
            {
                tokens.push_back(Token{TokenKind::Not, word});
            }
            else
            {
                tokens.push_back(Token{TokenKind::Term, std::string(m_input.substr(index, end - index))});
            }

            index = end;
        }

        tokens.push_back(Token{TokenKind::End, {}});

        return foundation::Result<std::vector<Token>>(std::move(tokens));
    }

    [[nodiscard]] const Token& current() const
    {
        return m_tokens[m_index];
    }

    void advance()
    {
        if (m_index + 1U < m_tokens.size())
        {
            ++m_index;
        }
    }

    [[nodiscard]] foundation::Result<SearchQuery> parseOrExpression()
    {
        auto leftResult = parseAndExpression();

        if (!leftResult)
        {
            return leftResult;
        }

        SearchQuery expression = std::move(*leftResult);

        while (current().kind == TokenKind::Or)
        {
            advance();
            auto rightResult = parseAndExpression();

            if (!rightResult)
            {
                return rightResult;
            }

            expression = SearchQuery::orQuery(std::move(expression), std::move(*rightResult));
        }

        return foundation::Result<SearchQuery>(std::move(expression));
    }

    [[nodiscard]] foundation::Result<SearchQuery> parseAndExpression()
    {
        auto leftResult = parseNotExpression();

        if (!leftResult)
        {
            return leftResult;
        }

        SearchQuery expression = std::move(*leftResult);

        while (current().kind == TokenKind::And)
        {
            advance();
            auto rightResult = parseNotExpression();

            if (!rightResult)
            {
                return rightResult;
            }

            expression = SearchQuery::andQuery(std::move(expression), std::move(*rightResult));
        }

        return foundation::Result<SearchQuery>(std::move(expression));
    }

    [[nodiscard]] foundation::Result<SearchQuery> parseNotExpression()
    {
        if (current().kind == TokenKind::Not)
        {
            advance();
            auto operandResult = parseNotExpression();

            if (!operandResult)
            {
                return operandResult;
            }

            return foundation::Result<SearchQuery>(SearchQuery::notQuery(std::move(*operandResult)));
        }

        return parsePrimaryExpression();
    }

    [[nodiscard]] foundation::Result<SearchQuery> parsePrimaryExpression()
    {
        if (current().kind == TokenKind::LeftParen)
        {
            advance();
            auto expressionResult = parseOrExpression();

            if (!expressionResult)
            {
                return expressionResult;
            }

            if (current().kind != TokenKind::RightParen)
            {
                return foundation::Result<SearchQuery>(foundation::Error(
                    foundation::ErrorCode::InvalidArgument, "Missing closing parenthesis in search query."));
            }

            advance();

            return expressionResult;
        }

        if (current().kind == TokenKind::Term)
        {
            SearchQuery query = SearchQuery::term(current().value);
            advance();

            return foundation::Result<SearchQuery>(std::move(query));
        }

        return foundation::Result<SearchQuery>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Expected search term in query."));
    }

    std::string_view m_input;
    std::vector<Token> m_tokens;
    std::size_t m_index{0U};
};

} // namespace

foundation::Result<SearchQuery> parseSearchQuery(const std::string_view expression) noexcept
{
    if (foundation::isBlank(expression))
    {
        return foundation::Result<SearchQuery>(SearchQuery::matchAll());
    }

    Parser parser(expression);

    return parser.parseExpression();
}

foundation::Result<SearchQuery> parseRegexQuery(const std::string_view pattern,
                                              const CaseSensitivity caseSensitivity) noexcept
{
    if (foundation::isBlank(pattern))
    {
        return foundation::Result<SearchQuery>(SearchQuery::matchAll());
    }

    if (pattern.size() > maxRegexPatternLength)
    {
        return foundation::Result<SearchQuery>(foundation::Error(
            foundation::ErrorCode::InvalidArgument,
            "Regex pattern exceeds maximum length of " + std::to_string(maxRegexPatternLength) + " characters."));
    }

    try
    {
        const std::regex testPattern(
            std::string(pattern), caseSensitivity == CaseSensitivity::Sensitive
                                      ? std::regex::ECMAScript
                                      : std::regex::ECMAScript | std::regex::icase);
        (void)testPattern;
    }
    catch (const std::regex_error& error)
    {
        return foundation::Result<SearchQuery>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, std::string("Invalid regex pattern: ") + error.what()));
    }

    return foundation::Result<SearchQuery>(SearchQuery::term(std::string(pattern), SearchMode::Regex, caseSensitivity));
}

} // namespace scope::search
