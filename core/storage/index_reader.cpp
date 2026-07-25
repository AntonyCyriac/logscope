/**
 * @file index_reader.cpp
 */

#include "index_reader.hpp"

#include "query_cache_key.hpp"
#include "sqlite_index_store.hpp"

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

foundation::Result<PushdownFetchResult>
IndexReader::linesMatchingPushdownFilter(const query::QueryNode& filterNode,
                                         const std::string& sqlWhereClause) const
{
    if (m_persistentStore == nullptr)
    {
        return foundation::Result<PushdownFetchResult>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "SQL pushdown requires a persistent index store."));
    }

    const auto sqliteStore = std::static_pointer_cast<SqliteIndexStore>(m_persistentStore);
    const auto fetched =
        sqliteStore->fetchLinesMatchingPushdown(canonicalFilterText(filterNode), sqlWhereClause);

    if (!fetched)
    {
        return foundation::Result<PushdownFetchResult>(fetched.error());
    }

    PushdownFetchResult result;
    result.cacheHit = fetched->cacheHit;
    result.lines = std::move(fetched->lines);

    return foundation::Result<PushdownFetchResult>(std::move(result));
}

foundation::Result<std::vector<analysis::IndexedLine>>
IndexReader::linesMatchingFtsSearch(const std::string& term) const
{
    if (m_persistentStore == nullptr)
    {
        return foundation::Result<std::vector<analysis::IndexedLine>>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "FTS search requires a persistent index store."));
    }

    const auto sqliteStore = std::static_pointer_cast<SqliteIndexStore>(m_persistentStore);

    return sqliteStore->fetchLinesMatchingFts(term);
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
