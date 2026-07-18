/**
 * @file filesystem.hpp
 * @brief Filesystem operations.
 */

#pragma once

#include <cstdint>

#include "path.hpp"
#include "result.hpp"

namespace scope::foundation
{

/**
 * @brief Provides filesystem operations built on Foundation paths.
 */
class FileSystem
{
  public:
    /**
     * @brief Determines whether a path exists.
     *
     * @param path Path to check.
     * @return true if the path exists.
     */
    [[nodiscard]] static Result<bool> exists(const Path& path);

    /**
     * @brief Determines whether a path refers to a regular file.
     *
     * @param path Path to check.
     * @return true if the path is a regular file.
     */
    [[nodiscard]] static Result<bool> isFile(const Path& path);

    /**
     * @brief Determines whether a path refers to a directory.
     *
     * @param path Path to check.
     * @return true if the path is a directory.
     */
    [[nodiscard]] static Result<bool> isDirectory(const Path& path);

    /**
     * @brief Returns the size of a regular file in bytes.
     *
     * @param path Path to the file.
     * @return File size in bytes.
     */
    [[nodiscard]] static Result<std::uint64_t> fileSize(const Path& path);
};

} // namespace scope::foundation
