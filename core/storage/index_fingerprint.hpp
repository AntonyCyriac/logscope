/**
 * @file index_fingerprint.hpp
 * @brief Source fingerprint for index cache invalidation.
 */

#pragma once

#include <string>

#include "foundation/path.hpp"
#include "foundation/result.hpp"

namespace scope::storage
{

/**
 * @brief Stable fingerprint for a log source file (path + size + mtime).
 */
class IndexFingerprint
{
  public:
    [[nodiscard]] const std::string& value() const noexcept;

    [[nodiscard]] static foundation::Result<IndexFingerprint> compute(const foundation::Path& sourcePath);

    [[nodiscard]] static IndexFingerprint fromStored(std::string value);

    [[nodiscard]] static foundation::Result<bool> matchesSource(const IndexFingerprint& fingerprint,
                                                                const foundation::Path& sourcePath);

  private:
    explicit IndexFingerprint(std::string value) noexcept;

    std::string m_value;
};

} // namespace scope::storage
