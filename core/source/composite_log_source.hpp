/**
 * @file composite_log_source.hpp
 * @brief Multi-file log source that reads sources sequentially.
 */

#pragma once

#include <memory>
#include <vector>

#include "foundation/path.hpp"
#include "foundation/result.hpp"
#include "log_source.hpp"

namespace scope::source
{

/**
 * @brief Reads log data from multiple sources in order.
 */
class CompositeLogSource : public LogSource
{
  public:
    /**
     * @brief Creates a composite source over one or more opened sources.
     *
     * @param displayPath Path shown in analysis metadata (for example, a directory).
     * @param sources Opened sources to read sequentially.
     * @return Composite source or error.
     */
    [[nodiscard]] static foundation::Result<std::unique_ptr<LogSource>> create(
        foundation::Path displayPath, std::vector<std::unique_ptr<LogSource>> sources);

    [[nodiscard]] const foundation::Path& path() const noexcept override;

    [[nodiscard]] foundation::Result<bool> readLine(std::string& line) override;

  private:
    CompositeLogSource(foundation::Path displayPath, std::vector<std::unique_ptr<LogSource>> sources);

    foundation::Path m_displayPath;
    std::vector<std::unique_ptr<LogSource>> m_sources;
    std::size_t m_currentIndex = 0;
};

} // namespace scope::source
