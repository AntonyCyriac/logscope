/**
 * @file index_store.hpp
 * @brief Persistent investigation index store interface.
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "foundation/path.hpp"
#include "foundation/result.hpp"
#include "index_fingerprint.hpp"
#include "line_index.hpp"
#include "log_format.hpp"

namespace scope::storage
{

struct IndexMetadata
{
    std::string fingerprint;
    foundation::Path sourcePath;
    analysis::LogFormat format{analysis::LogFormat::PlainText};
    std::uint64_t totalLines{0U};
};

/**
 * @brief Persistent store for analyzed log line metadata and content.
 */
class IndexStore
{
  public:
    virtual ~IndexStore() = default;

    [[nodiscard]] virtual foundation::Result<bool> appendLine(const analysis::IndexedLine& line,
                                                              std::string_view fullContent) = 0;

    [[nodiscard]] virtual foundation::Result<bool> finalize(std::uint64_t totalLines) = 0;

    [[nodiscard]] virtual std::uint64_t storedLineCount() const noexcept = 0;

    [[nodiscard]] virtual const foundation::Path& path() const noexcept = 0;

    [[nodiscard]] virtual const IndexMetadata& metadata() const noexcept = 0;

    [[nodiscard]] virtual foundation::Result<std::vector<analysis::IndexedLine>>
    fetchAllLines() const = 0;

    [[nodiscard]] virtual foundation::Result<std::vector<analysis::IndexedLine>>
    fetchLinesWhere(const std::string& sqlWhereClause) const = 0;
};

using IndexStorePtr = std::shared_ptr<IndexStore>;

} // namespace scope::storage
