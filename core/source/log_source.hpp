/**
 * @file log_source.hpp
 * @brief Abstract interface for reading log data.
 */

#pragma once

#include <string>

#include "foundation/path.hpp"
#include "foundation/result.hpp"

namespace scope::source
{

/**
 * @brief Provides sequential access to log data from a single source.
 */
class LogSource
{
  public:
    virtual ~LogSource() = default;

    /**
     * @brief Returns the path associated with this source.
     */
    [[nodiscard]] virtual const foundation::Path& path() const noexcept = 0;

    /**
     * @brief Reads the next line from the source.
     *
     * @param line Output line without the trailing newline.
     * @return true if a line was read, false at end of file, or error on failure.
     */
    [[nodiscard]] virtual foundation::Result<bool> readLine(std::string& line) = 0;
};

} // namespace scope::source
