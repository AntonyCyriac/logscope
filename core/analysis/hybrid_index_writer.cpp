/**
 * @file hybrid_index_writer.cpp
 */

#include "hybrid_index_writer.hpp"

#include "log_macros.hpp"

namespace scope::analysis
{

namespace
{

constexpr std::uint64_t persistProgressInterval = 10000U;

} // namespace

HybridIndexWriter::HybridIndexWriter(LineIndex lineIndex, storage::StorageConfig storageConfig,
                                     storage::IndexStorePtr persistentStore) noexcept
    : m_lineIndex(std::move(lineIndex))
    , m_storageConfig(std::move(storageConfig))
    , m_persistentStore(std::move(persistentStore))
{
}

bool HybridIndexWriter::tryAddLine(IndexedLine line, const std::string_view fullContent)
{
    const bool storedInMemory = m_lineIndex.tryAddLine(line);

    if (m_persistentStore != nullptr)
    {
        const bool spillToStore = m_storageConfig.mode == storage::StorageMode::Persistent ||
                                  m_storageConfig.persistIndex || !storedInMemory;

        if (spillToStore)
        {
            const auto appendResult = m_persistentStore->appendLine(line, fullContent);

            if (appendResult)
            {
                ++m_persistedLineCount;

                if (m_persistedLineCount % persistProgressInterval == 0U)
                {
                    SCOPE_LOG_INFO("analysis",
                                   "Indexed " + std::to_string(m_persistedLineCount) + " lines to persistent store");
                }
            }
        }
    }

    return storedInMemory || m_persistentStore != nullptr;
}

foundation::Result<bool> HybridIndexWriter::finalize(const std::uint64_t totalLines)
{
    if (m_persistentStore != nullptr)
    {
        return m_persistentStore->finalize(totalLines);
    }

    return foundation::Result<bool>(true);
}

LineIndex&& HybridIndexWriter::finishLineIndex() noexcept
{
    return std::move(m_lineIndex);
}

storage::IndexStorePtr HybridIndexWriter::indexStore() const noexcept
{
    return m_persistentStore;
}

} // namespace scope::analysis
