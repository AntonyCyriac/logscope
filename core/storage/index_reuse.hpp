/**
 * @file index_reuse.hpp
 * @brief Index reuse preparation for unchanged, append, and rebuild modes.
 */

#pragma once

#include <cstdint>

#include "foundation/path.hpp"
#include "foundation/result.hpp"
#include "index_fingerprint.hpp"
#include "index_store.hpp"
#include "storage_config.hpp"

namespace scope::storage
{

enum class IndexReuseMode
{
    Rebuild,
    Unchanged,
    Append
};

struct IndexReusePrepareResult
{
    IndexReuseMode mode{IndexReuseMode::Rebuild};
    IndexStorePtr store;
    std::uint64_t linesToSkip{0U};
};

[[nodiscard]] foundation::Result<IndexReusePrepareResult>
prepareIndexReuse(const StorageConfig& config, const IndexFingerprint& fingerprint,
                  const foundation::Path& sourcePath);

} // namespace scope::storage
