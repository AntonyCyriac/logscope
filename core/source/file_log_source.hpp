/**
 * @file file_log_source.hpp
 * @brief File-based log source implementation.
 */

#pragma once

#include <fstream>
#include <memory>

#include "foundation/path.hpp"
#include "foundation/result.hpp"
#include "log_source.hpp"

namespace scope::source
{

/**
 * @brief Reads log data from a file on disk.
 */
class FileLogSource : public LogSource
{
  public:
    /**
     * @brief Opens a file log source.
     *
     * @param path Path to the log file.
     * @return Opened source or error.
     */
    [[nodiscard]] static foundation::Result<std::unique_ptr<LogSource>> open(const foundation::Path& path);

    [[nodiscard]] const foundation::Path& path() const noexcept override;

    [[nodiscard]] foundation::Result<bool> readLine(std::string& line) override;

  private:
    explicit FileLogSource(foundation::Path path, std::ifstream stream);

    foundation::Path m_path;
    std::ifstream m_stream;
};

} // namespace scope::source
