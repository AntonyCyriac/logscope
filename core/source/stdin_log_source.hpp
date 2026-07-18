/**
 * @file stdin_log_source.hpp
 * @brief Standard input log source implementation.
 */

#pragma once

#include <iosfwd>
#include <istream>
#include <memory>

#include "foundation/path.hpp"
#include "foundation/result.hpp"
#include "log_source.hpp"

namespace scope::source
{

/**
 * @brief Reads log data from standard input.
 */
class StdinLogSource : public LogSource
{
  public:
    /**
     * @brief Opens a stdin log source.
     *
     * @return Opened source or error.
     */
    [[nodiscard]] static foundation::Result<std::unique_ptr<LogSource>> open();

    [[nodiscard]] const foundation::Path& path() const noexcept override;

    [[nodiscard]] foundation::Result<bool> readLine(std::string& line) override;

  private:
    explicit StdinLogSource(std::istream& input);

    static foundation::Path stdinPath();

    std::istream& m_input;
    foundation::Path m_path;
};

} // namespace scope::source
