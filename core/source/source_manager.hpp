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
     * @brief Validates that a log source exists and is supported.
     *
     * Accepts a file path, directory containing .log files, or "-" for stdin.
     *
     * @param path Path to the log source.
     * @return true if the source is valid.
     */
    [[nodiscard]] foundation::Result<bool> validate(const foundation::Path& path) const;

    /**
     * @brief Opens a log source and returns a prepared dataset.
     *
     * @param path Path to a file, directory, or "-" for stdin.
     * @return Source dataset or error.
     */
    [[nodiscard]] foundation::Result<SourceDataset> open(const foundation::Path& path) const;
};

} // namespace scope::source
