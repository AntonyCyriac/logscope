/**
 * @file source_manager.hpp
 * @brief Discovers, validates, and opens log sources.
 */

#pragma once

#include "foundation/path.hpp"
#include "foundation/result.hpp"
#include "source_dataset.hpp"

namespace scope::source
{

/**
 * @brief Manages the lifecycle of supported log sources.
 */
class SourceManager
{
  public:
    /**
     * @brief Validates that a file source exists and is readable.
     *
     * @param path Path to the log file.
     * @return true if the source is valid.
     */
    [[nodiscard]] foundation::Result<bool> validate(const foundation::Path& path) const;

    /**
     * @brief Opens a file source and returns a prepared dataset.
     *
     * @param path Path to the log file.
     * @return Source dataset or error.
     */
    [[nodiscard]] foundation::Result<SourceDataset> open(const foundation::Path& path) const;
};

} // namespace scope::source
