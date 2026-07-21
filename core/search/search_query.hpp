/**
 * @file search_query.hpp
 * @brief Search query representation for log line matching.
 */

#pragma once

#include <memory>
#include <string>

namespace scope::search
{

/// How a term is matched against line content.
enum class SearchMode
{
    Text,
    Regex
};

/// Whether matching respects character case.
enum class CaseSensitivity
{
    Insensitive,
    Sensitive
};

/**
 * @brief Structured search query supporting text terms and boolean composition.
 */
class SearchQuery
{
  public:
    enum class Kind
    {
        MatchAll,
        Term,
        And,
        Or,
        Not
    };

    SearchQuery();

    SearchQuery(SearchQuery&&) noexcept;
    SearchQuery& operator=(SearchQuery&&) noexcept;

    SearchQuery(const SearchQuery& other);
    SearchQuery& operator=(const SearchQuery& other);

    ~SearchQuery();

    [[nodiscard]] static SearchQuery matchAll() noexcept;

    [[nodiscard]] static SearchQuery term(std::string value, SearchMode mode = SearchMode::Text,
                                          CaseSensitivity caseSensitivity = CaseSensitivity::Insensitive);

    [[nodiscard]] static SearchQuery andQuery(SearchQuery left, SearchQuery right);

    [[nodiscard]] static SearchQuery orQuery(SearchQuery left, SearchQuery right);

    [[nodiscard]] static SearchQuery notQuery(SearchQuery operand);

    [[nodiscard]] Kind kind() const noexcept;

    [[nodiscard]] SearchMode mode() const noexcept;

    [[nodiscard]] CaseSensitivity caseSensitivity() const noexcept;

    [[nodiscard]] const std::string& term() const noexcept;

    [[nodiscard]] const SearchQuery* left() const noexcept;

    [[nodiscard]] const SearchQuery* right() const noexcept;

    [[nodiscard]] const SearchQuery* operand() const noexcept;

    [[nodiscard]] bool isActive() const noexcept;

    [[nodiscard]] std::string toString() const;

  private:
    explicit SearchQuery(Kind kind);

    Kind m_kind{Kind::MatchAll};
    SearchMode m_mode{SearchMode::Text};
    CaseSensitivity m_caseSensitivity{CaseSensitivity::Insensitive};
    std::string m_term;
    std::unique_ptr<SearchQuery> m_left;
    std::unique_ptr<SearchQuery> m_right;
    std::unique_ptr<SearchQuery> m_operand;
};

} // namespace scope::search
