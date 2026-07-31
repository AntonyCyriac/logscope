/**
 * @file tailing_file_log_source.hpp
 * @brief File log source with optional live tail (follow) mode.
 */

#pragma once

#include <chrono>
#include <cstdint>
#include <fstream>
#include <memory>
#include <thread>

#include "foundation/path.hpp"
#include "foundation/result.hpp"
#include "log_source.hpp"

namespace scope::source
{

/**
 * @brief Reads a file and optionally waits for appended lines (tail -f).
 */
class TailingFileLogSource : public LogSource
{
  public:
    /**
     * @brief Opens a file for reading.
     *
     * @param path File path.
     * @param follow When true, blocks polling after EOF until new data or stop.
     */
    [[nodiscard]] static foundation::Result<std::unique_ptr<LogSource>> open(const foundation::Path& path,
                                                                             bool follow);

    /**
     * @brief Opens a file positioned at end for tail-only reading.
     */
    [[nodiscard]] static foundation::Result<std::unique_ptr<LogSource>> openAtEnd(const foundation::Path& path);

    [[nodiscard]] const foundation::Path& path() const noexcept override;

    [[nodiscard]] foundation::Result<bool> readLine(std::string& line) override;

    /**
     * @brief Stops follow polling (tail toggle off).
     */
    void stopFollow() noexcept;

    [[nodiscard]] bool isFollowing() const noexcept;

    /**
     * @brief Reads one appended line without blocking for new data (HTTP tail poll).
     */
    [[nodiscard]] foundation::Result<bool> pollLine(std::string& line);

  private:
    TailingFileLogSource(foundation::Path path, std::ifstream stream, bool follow, std::uint64_t readPosition);

    [[nodiscard]] foundation::Result<bool> tryReadLine(std::string& line);

    void reopenFromPosition(std::uint64_t position);

    foundation::Path m_path;
    std::ifstream m_stream;
    bool m_follow;
    bool m_stopped{false};
    std::uint64_t m_readPosition{0U};
};

} // namespace scope::source
