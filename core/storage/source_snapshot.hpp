/**
 * @file source_snapshot.hpp
 * @brief Source file snapshot comparison for incremental index reuse.
 */

#pragma once

#include <cstdint>
#include <string>

#include "foundation/path.hpp"
#include "foundation/result.hpp"

struct sqlite3;

namespace scope::storage
{

struct StoredSourceSnapshot
{
    std::string sourcePath;
    std::uint64_t sourceSize{0U};
    std::int64_t sourceMtime{0};
    std::uint64_t totalLines{0U};
    std::uint64_t indexedLineCount{0U};
    std::string sourceContentSha256;
    std::string sourcePrefixSha256;
};

enum class SourceChangeKind
{
    Unknown,
    Unchanged,
    Grown,
    Truncated
};

[[nodiscard]] foundation::Result<StoredSourceSnapshot> readStoredSourceSnapshot(sqlite3* database);

[[nodiscard]] foundation::Result<SourceChangeKind> compareSourceChange(const StoredSourceSnapshot& stored,
                                                                       const foundation::Path& sourcePath);

} // namespace scope::storage
