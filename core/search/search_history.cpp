/**
 * @file search_history.cpp
 * @brief SearchHistory implementation.
 */

#include "search_history.hpp"

#include "foundation/string.hpp"

namespace scope::search
{

void SearchHistory::add(const std::string query)
{
    if (foundation::isBlank(query))
    {
        return;
    }

    for (const std::string& entry : m_entries)
    {
        if (entry == query)
        {
            return;
        }
    }

    if (m_entries.size() >= maxSearchHistoryEntries)
    {
        m_entries.erase(m_entries.begin());
    }

    m_entries.push_back(query);
}

const std::vector<std::string>& SearchHistory::entries() const noexcept
{
    return m_entries;
}

std::string SearchHistory::serialize() const
{
    std::string serialized;

    for (std::size_t index = 0U; index < m_entries.size(); ++index)
    {
        if (index > 0U)
        {
            serialized.push_back(';');
        }

        serialized += m_entries[index];
    }

    return serialized;
}

SearchHistory SearchHistory::deserialize(const std::string_view value)
{
    SearchHistory history;

    if (foundation::isBlank(value))
    {
        return history;
    }

    const std::vector<std::string> entries = foundation::split(value, ';');

    for (const std::string& entry : entries)
    {
        history.add(entry);
    }

    return history;
}

} // namespace scope::search
