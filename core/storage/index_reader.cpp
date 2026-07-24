/**
 * @file index_reader.cpp
 */

#include "index_reader.hpp"

namespace scope::storage
{

IndexReader::IndexReader(const analysis::LineIndex* memoryIndex, IndexStorePtr persistentStore) noexcept
    : m_memoryIndex(memoryIndex)
    , m_persistentStore(std::move(persistentStore))
{
}

std::vector<analysis::IndexedLine> IndexReader::lines() const
{
    if (m_persistentStore != nullptr && m_persistentStore->storedLineCount() > 0U)
    {
        const auto fetched = m_persistentStore->fetchAllLines();

        if (fetched)
        {
            return *fetched;
        }
    }

    if (m_memoryIndex != nullptr)
    {
        return m_memoryIndex->lines();
    }

    return {};
}

foundation::Result<std::vector<analysis::IndexedLine>>
IndexReader::linesMatchingWhere(const std::string& sqlWhereClause) const
{
    if (m_persistentStore != nullptr)
    {
        return m_persistentStore->fetchLinesWhere(sqlWhereClause);
    }

    return foundation::Result<std::vector<analysis::IndexedLine>>(
        foundation::Error(foundation::ErrorCode::InvalidArgument,
                          "SQL pushdown requires a persistent index store."));
}

std::uint64_t IndexReader::indexedLineCount() const noexcept
{
    if (m_persistentStore != nullptr && m_persistentStore->storedLineCount() > 0U)
    {
        return m_persistentStore->storedLineCount();
    }

    if (m_memoryIndex != nullptr)
    {
        return m_memoryIndex->indexedLineCount();
    }

    return 0U;
}

std::uint64_t IndexReader::truncatedLineCount() const noexcept
{
    if (m_persistentStore != nullptr && m_persistentStore->storedLineCount() > 0U)
    {
        if (m_memoryIndex != nullptr && m_persistentStore->storedLineCount() > m_memoryIndex->indexedLineCount())
        {
            return m_persistentStore->storedLineCount() - m_memoryIndex->indexedLineCount();
        }

        return 0U;
    }

    if (m_memoryIndex != nullptr)
    {
        return m_memoryIndex->truncatedLineCount();
    }

    return 0U;
}

bool IndexReader::hasPersistentStore() const noexcept
{
    return m_persistentStore != nullptr;
}

const IndexStore* IndexReader::persistentStore() const noexcept
{
    return m_persistentStore.get();
}

} // namespace scope::storage
