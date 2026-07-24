/**
 * @file index_store_factory.hpp
 * @brief Factory helpers for persistent investigation indexes.
 */

#pragma once

#include "foundation/path.hpp"
#include "foundation/result.hpp"
#include "index_fingerprint.hpp"
#include "index_store.hpp"
#include "log_format.hpp"
#include "storage_config.hpp"

namespace scope::storage
{

[[nodiscard]] foundation::Path resolveIndexPath(const StorageConfig& config,
                                                const IndexFingerprint& fingerprint);

[[nodiscard]] foundation::Result<IndexStorePtr>
createIndexStore(const StorageConfig& config, const IndexFingerprint& fingerprint,
                 const foundation::Path& sourcePath, analysis::LogFormat format);

[[nodiscard]] foundation::Result<IndexStorePtr>
tryOpenReusableIndex(const StorageConfig& config, const IndexFingerprint& fingerprint,
                     const foundation::Path& sourcePath);

} // namespace scope::storage
