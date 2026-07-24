/**
 * @file hybrid_index_writer.hpp
 * @brief Hybrid in-memory and persistent line index writer.
 */

#pragma once

#include <cstdint>
#include <string_view>

#include "foundation/result.hpp"
#include "index_store.hpp"
#include "line_index.hpp"
#include "storage_config.hpp"

namespace scope::analysis
{

/**
 * @brief Writes indexed lines to memory and optionally to a persistent store.
 */
class HybridIndexWriter
{
  public:
    HybridIndexWriter(LineIndex lineIndex, storage::StorageConfig storageConfig,
                      storage::IndexStorePtr persistentStore) noexcept;

    [[nodiscard]] bool tryAddLine(IndexedLine line, std::string_view fullContent);

    [[nodiscard]] foundation::Result<bool> finalize(std::uint64_t totalLines);

    [[nodiscard]] LineIndex&& finishLineIndex() noexcept;

    [[nodiscard]] storage::IndexStorePtr indexStore() const noexcept;

  private:
    LineIndex m_lineIndex;
    storage::StorageConfig m_storageConfig;
    storage::IndexStorePtr m_persistentStore;
    std::uint64_t m_persistedLineCount{0U};
};

} // namespace scope::analysis
