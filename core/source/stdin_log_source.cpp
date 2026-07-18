/**
 * @file stdin_log_source.cpp
 * @brief StdinLogSource implementation.
 */

#include "stdin_log_source.hpp"

#include <iostream>

#include "foundation/error.hpp"
#include "log_macros.hpp"

namespace scope::source
{

StdinLogSource::StdinLogSource(std::istream& input) : m_input(input), m_path(stdinPath())
{
}

foundation::Path StdinLogSource::stdinPath()
{
    return foundation::Path("-");
}

foundation::Result<std::unique_ptr<LogSource>> StdinLogSource::open()
{
    SCOPE_LOG_DEBUG("source.stdin", "Opened standard input log source");

    return foundation::Result<std::unique_ptr<LogSource>>(
        std::unique_ptr<LogSource>(new StdinLogSource(std::cin)));
}

const foundation::Path& StdinLogSource::path() const noexcept
{
    return m_path;
}

foundation::Result<bool> StdinLogSource::readLine(std::string& line)
{
    if (!std::getline(m_input, line))
    {
        if (m_input.eof())
        {
            return foundation::Result<bool>(false);
        }

        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::IOError, "Failed to read from standard input."));
    }

    return foundation::Result<bool>(true);
}

} // namespace scope::source
