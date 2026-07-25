/**
 * @file search_provider.hpp
 * @brief Plugin search provider interface (M12).
 */

#pragma once

#include <string>
#include <vector>

#include "line_index.hpp"
#include "search_query.hpp"

namespace scope::search
{

/**
 * @brief Optional plugin-provided search implementation.
 */
class SearchProvider
{
  public:
    virtual ~SearchProvider() = default;

    [[nodiscard]] virtual std::string id() const = 0;

    [[nodiscard]] virtual std::vector<analysis::IndexedLine>
    search(const analysis::LineIndex& index, const SearchQuery& query) const = 0;
};

} // namespace scope::search
