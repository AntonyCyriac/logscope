/**
 * @file index_reader.hpp
 * @brief Unified read path over memory and persistent indexes.
 */

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "index_store.hpp"
#include "line_index.hpp"

namespace scope::storage
{

/**
 * @brief Read-only view over an in-memory line index and optional persistent store.
 */
class IndexReader
{
  public:
    IndexReader(const analysis::LineIndex* memoryIndex, IndexStorePtr persistentStore) noexcept;

    [[nodiscard]] std::vector<analysis::IndexedLine> lines() const;

    [[nodiscard]] foundation::Result<std::vector<analysis::IndexedLine>>
    linesMatchingWhere(const std::string& sqlWhereClause) const;

    [[nodiscard]] std::uint64_t indexedLineCount() const noexcept;

    [[nodiscard]] std::uint64_t truncatedLineCount() const noexcept;

    [[nodiscard]] bool hasPersistentStore() const noexcept;

    [[nodiscard]] const IndexStore* persistentStore() const noexcept;

  private:
    const analysis::LineIndex* m_memoryIndex{nullptr};
    IndexStorePtr m_persistentStore;
};

} // namespace scope::storage
