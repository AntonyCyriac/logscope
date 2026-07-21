/**
 * @file search_history.hpp
 * @brief Bounded search query history for progressive investigation.
 */

#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace scope::search
{

/// Maximum number of search queries retained in history.
constexpr std::size_t maxSearchHistoryEntries = 20U;

/**
 * @brief Stores recent search queries in insertion order.
 */
class SearchHistory
{
  public:
    void add(std::string query);

    [[nodiscard]] const std::vector<std::string>& entries() const noexcept;

    [[nodiscard]] std::string serialize() const;

    [[nodiscard]] static SearchHistory deserialize(std::string_view value);

  private:
    std::vector<std::string> m_entries;
};

} // namespace scope::search
