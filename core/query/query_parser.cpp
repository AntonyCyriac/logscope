/**
 * @file query_parser.cpp
 */

#include "query_parser.hpp"

#include <cctype>
#include <optional>
#include <string>
#include <vector>

#include "foundation/error.hpp"
#include "foundation/string.hpp"

namespace scope::query
{

namespace
{

enum class TokenKind
{
    End,
    Ident,
    String,
    Number,
    And,
    Or,
    Not,
    LeftParen,
    RightParen,
    Comma,
    Equal,
    NotEqual,
    Greater,
    GreaterEqual,
    Less,
    LessEqual
};

struct Token
{
    TokenKind kind{TokenKind::End};
    std::string value;
    std::size_t line{1U};
    std::size_t column{1U};
};

class Parser
{
  public:
    explicit Parser(std::string_view input) : m_input(input) {}

    [[nodiscard]] foundation::Result<QueryNode> parseExpression()
    {
        const auto tokensResult = tokenize();

        if (!tokensResult)
        {
            return foundation::Result<QueryNode>(tokensResult.error());
        }

        m_tokens = *tokensResult;
        m_index = 0U;

        if (m_tokens.empty() || (m_tokens.size() == 1U && m_tokens[0].kind == TokenKind::End))
        {
            return foundation::Result<QueryNode>(QueryNode::matchAll());
        }

        auto expressionResult = parseOrExpression();

        if (!expressionResult)
        {
            return expressionResult;
        }

        if (current().kind != TokenKind::End)
        {
            return makeError("Unexpected token in filter query.");
        }

        return expressionResult;
    }

  private:
    [[nodiscard]] foundation::Result<std::vector<Token>> tokenize()
    {
        std::vector<Token> tokens;
        std::size_t index = 0U;
        std::size_t line = 1U;
        std::size_t column = 1U;

        while (index < m_input.size())
        {
            const char character = m_input[index];

            if (std::isspace(static_cast<unsigned char>(character)) != 0)
            {
                if (character == '\n')
                {
                    ++line;
                    column = 1U;
                }
                else
                {
                    ++column;
                }

                ++index;
                continue;
            }

            if (character == '(')
            {
                tokens.push_back(Token{TokenKind::LeftParen, "(", line, column});
                ++index;
                ++column;
                continue;
            }

            if (character == ')')
            {
                tokens.push_back(Token{TokenKind::RightParen, ")", line, column});
                ++index;
                ++column;
                continue;
            }

            if (character == ',')
            {
                tokens.push_back(Token{TokenKind::Comma, ",", line, column});
                ++index;
                ++column;
                continue;
            }

            if (character == '"')
            {
                const std::size_t startColumn = column;
                ++index;
                ++column;
                std::string value;
                bool closed = false;

                while (index < m_input.size())
                {
                    const char currentCharacter = m_input[index];

                    if (currentCharacter == '\\' && index + 1U < m_input.size())
                    {
                        value.push_back(m_input[index + 1U]);
                        index += 2U;
                        column += 2U;
                        continue;
                    }

                    if (currentCharacter == '"')
                    {
                        closed = true;
                        ++index;
                        ++column;
                        break;
                    }

                    if (currentCharacter == '\n')
                    {
                        ++line;
                        column = 1U;
                    }
                    else
                    {
                        ++column;
                    }

                    value.push_back(currentCharacter);
                    ++index;
                }

                if (!closed)
                {
                    return foundation::Result<std::vector<Token>>(foundation::Error(
                        foundation::ErrorCode::InvalidArgument,
                        formatPosition(line, startColumn) + "Unterminated string literal."));
                }

                tokens.push_back(Token{TokenKind::String, std::move(value), line, startColumn});
                continue;
            }

            if (std::isdigit(static_cast<unsigned char>(character)) != 0)
            {
                const std::size_t startColumn = column;
                std::string value;

                while (index < m_input.size() &&
                       std::isdigit(static_cast<unsigned char>(m_input[index])) != 0)
                {
                    value.push_back(m_input[index]);
                    ++index;
                    ++column;
                }

                tokens.push_back(Token{TokenKind::Number, std::move(value), line, startColumn});
                continue;
            }

            if (character == '=' && index + 1U < m_input.size() && m_input[index + 1U] == '=')
            {
                tokens.push_back(Token{TokenKind::Equal, "==", line, column});
                index += 2U;
                column += 2U;
                continue;
            }

            if (character == '!' && index + 1U < m_input.size() && m_input[index + 1U] == '=')
            {
                tokens.push_back(Token{TokenKind::NotEqual, "!=", line, column});
                index += 2U;
                column += 2U;
                continue;
            }

            if (character == '>' && index + 1U < m_input.size() && m_input[index + 1U] == '=')
            {
                tokens.push_back(Token{TokenKind::GreaterEqual, ">=", line, column});
                index += 2U;
                column += 2U;
                continue;
            }

            if (character == '<' && index + 1U < m_input.size() && m_input[index + 1U] == '=')
            {
                tokens.push_back(Token{TokenKind::LessEqual, "<=", line, column});
                index += 2U;
                column += 2U;
                continue;
            }

            if (character == '>')
            {
                tokens.push_back(Token{TokenKind::Greater, ">", line, column});
                ++index;
                ++column;
                continue;
            }

            if (character == '<')
            {
                tokens.push_back(Token{TokenKind::Less, "<", line, column});
                ++index;
                ++column;
                continue;
            }

            if (std::isalpha(static_cast<unsigned char>(character)) != 0 || character == '_')
            {
                const std::size_t startColumn = column;
                std::string value;

                while (index < m_input.size())
                {
                    const char currentCharacter = m_input[index];

                    if (std::isalnum(static_cast<unsigned char>(currentCharacter)) == 0 && currentCharacter != '_')
                    {
                        break;
                    }

                    value.push_back(currentCharacter);
                    ++index;
                    ++column;
                }

                const std::string lowered = foundation::toLower(value);

                if (lowered == "and")
                {
                    tokens.push_back(Token{TokenKind::And, value, line, startColumn});
                }
                else if (lowered == "or")
                {
                    tokens.push_back(Token{TokenKind::Or, value, line, startColumn});
                }
                else if (lowered == "not")
                {
                    tokens.push_back(Token{TokenKind::Not, value, line, startColumn});
                }
                else
                {
                    tokens.push_back(Token{TokenKind::Ident, value, line, startColumn});
                }

                continue;
            }

            return foundation::Result<std::vector<Token>>(foundation::Error(
                foundation::ErrorCode::InvalidArgument,
                formatPosition(line, column) + "Unexpected character in filter query."));
        }

        tokens.push_back(Token{TokenKind::End, "", line, column});

        return foundation::Result<std::vector<Token>>(std::move(tokens));
    }

    [[nodiscard]] foundation::Result<QueryNode> parseOrExpression()
    {
        auto leftResult = parseAndExpression();

        if (!leftResult)
        {
            return leftResult;
        }

        QueryNode left = std::move(*leftResult);

        while (current().kind == TokenKind::Or)
        {
            advance();
            auto rightResult = parseAndExpression();

            if (!rightResult)
            {
                return rightResult;
            }

            left = QueryNode::orNode(std::move(left), std::move(*rightResult));
        }

        return foundation::Result<QueryNode>(std::move(left));
    }

    [[nodiscard]] foundation::Result<QueryNode> parseAndExpression()
    {
        auto leftResult = parseUnaryExpression();

        if (!leftResult)
        {
            return leftResult;
        }

        QueryNode left = std::move(*leftResult);

        while (current().kind == TokenKind::And)
        {
            advance();
            auto rightResult = parseUnaryExpression();

            if (!rightResult)
            {
                return rightResult;
            }

            left = QueryNode::andNode(std::move(left), std::move(*rightResult));
        }

        return foundation::Result<QueryNode>(std::move(left));
    }

    [[nodiscard]] foundation::Result<QueryNode> parseUnaryExpression()
    {
        if (current().kind == TokenKind::Not)
        {
            advance();
            auto operandResult = parseUnaryExpression();

            if (!operandResult)
            {
                return operandResult;
            }

            return foundation::Result<QueryNode>(QueryNode::notNode(std::move(*operandResult)));
        }

        return parsePrimaryExpression();
    }

    [[nodiscard]] foundation::Result<QueryNode> parsePrimaryExpression()
    {
        if (current().kind == TokenKind::LeftParen)
        {
            advance();
            auto innerResult = parseOrExpression();

            if (!innerResult)
            {
                return innerResult;
            }

            if (current().kind != TokenKind::RightParen)
            {
                return makeError("Expected ')' after grouped expression.");
            }

            advance();

            return innerResult;
        }

        if (current().kind == TokenKind::Ident)
        {
            const std::string identifier = current().value;
            const std::string lowered = foundation::toLower(identifier);

            if (lowered == "contains")
            {
                return parseContainsCall();
            }

            if (lowered == "haskey")
            {
                return parseHasKeyCall();
            }

            advance();

            return parseComparison(identifier);
        }

        return makeError("Expected field comparison or function call.");
    }

    [[nodiscard]] foundation::Result<QueryNode> parseComparison(const std::string& field)
    {
        const Token operatorToken = current();
        ComparisonOperator op{};

        if (!parseComparisonOperator(op))
        {
            return makeError("Expected comparison operator after field name.");
        }

        const auto literalResult = parseLiteral();

        if (!literalResult)
        {
            return foundation::Result<QueryNode>(literalResult.error());
        }

        return foundation::Result<QueryNode>(QueryNode::comparison(field, op, std::move(*literalResult)));
    }

    [[nodiscard]] bool parseComparisonOperator(ComparisonOperator& op)
    {
        switch (current().kind)
        {
        case TokenKind::Equal:
            op = ComparisonOperator::Equal;
            advance();
            return true;
        case TokenKind::NotEqual:
            op = ComparisonOperator::NotEqual;
            advance();
            return true;
        case TokenKind::Greater:
            op = ComparisonOperator::Greater;
            advance();
            return true;
        case TokenKind::GreaterEqual:
            op = ComparisonOperator::GreaterEqual;
            advance();
            return true;
        case TokenKind::Less:
            op = ComparisonOperator::Less;
            advance();
            return true;
        case TokenKind::LessEqual:
            op = ComparisonOperator::LessEqual;
            advance();
            return true;
        default:
            return false;
        }
    }

    [[nodiscard]] foundation::Result<QueryNode> parseContainsCall()
    {
        if (current().kind != TokenKind::Ident || foundation::toLower(current().value) != "contains")
        {
            return makeError("Expected contains function.");
        }

        advance();

        if (current().kind != TokenKind::LeftParen)
        {
            return makeError("Expected '(' after contains.");
        }

        advance();

        if (current().kind != TokenKind::Ident)
        {
            return makeError("Expected field name in contains().");
        }

        const std::string field = current().value;
        advance();

        if (current().kind != TokenKind::Comma)
        {
            return makeError("Expected ',' in contains().");
        }

        advance();

        if (current().kind != TokenKind::String)
        {
            return makeError("Expected string argument in contains().");
        }

        const std::string argument = current().value;
        advance();

        if (current().kind != TokenKind::RightParen)
        {
            return makeError("Expected ')' after contains().");
        }

        advance();

        return foundation::Result<QueryNode>(
            QueryNode::functionCall(FunctionKind::Contains, field, argument));
    }

    [[nodiscard]] foundation::Result<QueryNode> parseHasKeyCall()
    {
        if (current().kind != TokenKind::Ident || foundation::toLower(current().value) != "haskey")
        {
            return makeError("Expected hasKey function.");
        }

        advance();

        if (current().kind != TokenKind::LeftParen)
        {
            return makeError("Expected '(' after hasKey.");
        }

        advance();

        if (current().kind != TokenKind::String && current().kind != TokenKind::Ident)
        {
            return makeError("Expected key name in hasKey().");
        }

        const std::string argument = current().value;
        advance();

        if (current().kind != TokenKind::RightParen)
        {
            return makeError("Expected ')' after hasKey().");
        }

        advance();

        return foundation::Result<QueryNode>(QueryNode::functionCall(FunctionKind::HasKey, "", argument));
    }

    [[nodiscard]] foundation::Result<QueryValue> parseLiteral()
    {
        if (current().kind == TokenKind::String)
        {
            QueryValue value = QueryValue::stringValue(current().value);
            advance();

            return foundation::Result<QueryValue>(std::move(value));
        }

        if (current().kind == TokenKind::Number)
        {
            QueryValue value = QueryValue::numberValue(static_cast<std::uint64_t>(std::stoull(current().value)));
            advance();

            return foundation::Result<QueryValue>(std::move(value));
        }

        if (current().kind == TokenKind::Ident)
        {
            const auto level = parseLevelKeyword(current().value);

            if (!level)
            {
                return foundation::Result<QueryValue>(foundation::Error(
                    foundation::ErrorCode::InvalidArgument,
                    formatPosition(current().line, current().column) + "Unknown level keyword."));
            }

            advance();

            return foundation::Result<QueryValue>(QueryValue::levelValue(*level));
        }

        return makeValueError("Expected literal value.");
    }

    [[nodiscard]] std::optional<analysis::DetectedLogLevel> parseLevelKeyword(const std::string& value) const
    {
        const std::string lowered = foundation::toLower(value);

        if (lowered == "error")
        {
            return analysis::DetectedLogLevel::Error;
        }

        if (lowered == "warn" || lowered == "warning")
        {
            return analysis::DetectedLogLevel::Warn;
        }

        if (lowered == "info")
        {
            return analysis::DetectedLogLevel::Info;
        }

        if (lowered == "blank")
        {
            return analysis::DetectedLogLevel::Blank;
        }

        if (lowered == "other")
        {
            return analysis::DetectedLogLevel::Other;
        }

        return std::nullopt;
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

    [[nodiscard]] std::string formatPosition(const std::size_t line, const std::size_t column) const
    {
        return std::to_string(line) + ':' + std::to_string(column) + ": ";
    }

    [[nodiscard]] foundation::Result<QueryNode> makeError(const std::string& message) const
    {
        return foundation::Result<QueryNode>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, formatPosition(current().line, current().column) + message));
    }

    [[nodiscard]] foundation::Result<QueryValue> makeValueError(const std::string& message) const
    {
        return foundation::Result<QueryValue>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, formatPosition(current().line, current().column) + message));
    }

    std::string_view m_input;
    std::vector<Token> m_tokens;
    std::size_t m_index{0U};
};

} // namespace

foundation::Result<QueryNode> parseFilterQuery(const std::string_view input)
{
    Parser parser(input);

    return parser.parseExpression();
}

} // namespace scope::query
