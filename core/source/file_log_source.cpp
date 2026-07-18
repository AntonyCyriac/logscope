/**
 * @file file_log_source.cpp
 * @brief FileLogSource implementation.
 */

#include "file_log_source.hpp"

#include "foundation/error.hpp"
#include "log_macros.hpp"

namespace scope::source
{

FileLogSource::FileLogSource(foundation::Path path, std::ifstream stream)
    : m_path(std::move(path)), m_stream(std::move(stream))
{
}

foundation::Result<std::unique_ptr<LogSource>> FileLogSource::open(const foundation::Path& path)
{
    std::ifstream stream(path.string());

    if (!stream)
    {
        SCOPE_LOG_ERROR("source.file", "Unable to open log file: " + path.string());

        return foundation::Result<std::unique_ptr<LogSource>>(
            foundation::Error(foundation::ErrorCode::IOError, "Unable to open log file."));
    }

    SCOPE_LOG_DEBUG("source.file", "Opened log file: " + path.string());

    return foundation::Result<std::unique_ptr<LogSource>>(
        std::unique_ptr<LogSource>(new FileLogSource(path, std::move(stream))));
}

const foundation::Path& FileLogSource::path() const noexcept
{
    return m_path;
}

foundation::Result<bool> FileLogSource::readLine(std::string& line)
{
    if (!std::getline(m_stream, line))
    {
        if (m_stream.eof())
        {
            return foundation::Result<bool>(false);
        }

        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::IOError, "Failed to read from log file."));
    }

    return foundation::Result<bool>(true);
}

} // namespace scope::source
