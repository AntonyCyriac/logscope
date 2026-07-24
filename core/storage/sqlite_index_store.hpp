/**
 * @file sqlite_index_store.hpp
 * @brief SQLite-backed IndexStore implementation.
 */

#pragma once

#include <memory>

#include "foundation/result.hpp"
#include "index_store.hpp"
#include "log_format.hpp"

namespace scope::storage
{

class SqliteIndexStore final : public IndexStore
{
  public:
    ~SqliteIndexStore() override;

    [[nodiscard]] static foundation::Result<IndexStorePtr>
    create(const foundation::Path& databasePath, const IndexMetadata& metadata);

    [[nodiscard]] static foundation::Result<IndexStorePtr> open(const foundation::Path& databasePath);

    [[nodiscard]] foundation::Result<bool> appendLine(const analysis::IndexedLine& line,
                                                      std::string_view fullContent) override;

    [[nodiscard]] foundation::Result<bool> finalize(std::uint64_t totalLines) override;

    [[nodiscard]] std::uint64_t storedLineCount() const noexcept override;

    [[nodiscard]] const foundation::Path& path() const noexcept override;

    [[nodiscard]] const IndexMetadata& metadata() const noexcept override;

    [[nodiscard]] foundation::Result<std::vector<analysis::IndexedLine>> fetchAllLines() const override;

    [[nodiscard]] foundation::Result<std::vector<analysis::IndexedLine>>
    fetchLinesWhere(const std::string& sqlWhereClause) const override;

  private:
    struct Impl;

    explicit SqliteIndexStore(std::unique_ptr<Impl> impl) noexcept;

    [[nodiscard]] foundation::Result<bool> beginWriteBatch();

    [[nodiscard]] foundation::Result<bool> commitWriteBatch();

    void rollbackWriteBatch() noexcept;

    [[nodiscard]] foundation::Result<bool> bindAndInsertLine(const analysis::IndexedLine& line,
                                                             std::string_view fullContent);

    std::unique_ptr<Impl> m_impl;
};

} // namespace scope::storage
