/**
 * @file tailing_file_log_source.cpp
 * @brief TailingFileLogSource implementation.
 */

#include "tailing_file_log_source.hpp"

#include "foundation/error.hpp"
#include "foundation/filesystem.hpp"
#include "log_macros.hpp"

namespace scope::source
{

namespace
{

constexpr auto kTailPollInterval = std::chrono::milliseconds(200);
constexpr int kTailPollAttempts = 25;

} // namespace

TailingFileLogSource::TailingFileLogSource(foundation::Path path, std::ifstream stream, bool follow,
                                           std::uint64_t readPosition)
    : m_path(std::move(path)), m_stream(std::move(stream)), m_follow(follow), m_readPosition(readPosition)
{
}

foundation::Result<std::unique_ptr<LogSource>> TailingFileLogSource::open(const foundation::Path& path, bool follow)
{
    std::ifstream stream(path.string());

    if (!stream)
    {
        SCOPE_LOG_ERROR("source.tail", "Unable to open log file: " + path.string());

        return foundation::Result<std::unique_ptr<LogSource>>(
            foundation::Error(foundation::ErrorCode::IOError, "Unable to open log file."));
    }

  return foundation::Result<std::unique_ptr<LogSource>>(
        std::unique_ptr<LogSource>(new TailingFileLogSource(path, std::move(stream), follow, 0U)));
}

foundation::Result<std::unique_ptr<LogSource>> TailingFileLogSource::openAtEnd(const foundation::Path& path)
{
    const auto sizeResult = foundation::FileSystem::fileSize(path);

    if (!sizeResult)
    {
        return foundation::Result<std::unique_ptr<LogSource>>(sizeResult.error());
    }

    std::ifstream stream(path.string());

    if (!stream)
    {
        return foundation::Result<std::unique_ptr<LogSource>>(
            foundation::Error(foundation::ErrorCode::IOError, "Unable to open log file."));
    }

    stream.seekg(static_cast<std::streamoff>(*sizeResult));

    return foundation::Result<std::unique_ptr<LogSource>>(
        std::unique_ptr<LogSource>(new TailingFileLogSource(path, std::move(stream), true, *sizeResult)));
}

const foundation::Path& TailingFileLogSource::path() const noexcept
{
    return m_path;
}

void TailingFileLogSource::stopFollow() noexcept
{
    m_stopped = true;
}

bool TailingFileLogSource::isFollowing() const noexcept
{
    return m_follow && !m_stopped;
}

void TailingFileLogSource::reopenFromPosition(const std::uint64_t position)
{
    m_stream.close();
    m_stream.open(m_path.string());

    if (m_stream)
    {
        m_stream.seekg(static_cast<std::streamoff>(position));
        m_readPosition = position;
    }
}

foundation::Result<bool> TailingFileLogSource::tryReadLine(std::string& line)
{
    if (!m_stream)
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::IOError, "Log file stream is not open."));
    }

    if (std::getline(m_stream, line))
    {
        m_readPosition = static_cast<std::uint64_t>(m_stream.tellg());

        return foundation::Result<bool>(true);
    }

    if (m_stream.eof())
    {
        return foundation::Result<bool>(false);
    }

    return foundation::Result<bool>(
        foundation::Error(foundation::ErrorCode::IOError, "Failed to read from log file."));
}

foundation::Result<bool> TailingFileLogSource::readLine(std::string& line)
{
    const auto readResult = tryReadLine(line);

    if (!readResult)
    {
        return readResult;
    }

    if (*readResult)
    {
        return readResult;
    }

    if (!m_follow || m_stopped)
    {
        return foundation::Result<bool>(false);
    }

    for (int attempt = 0; attempt < kTailPollAttempts; ++attempt)
    {
        if (m_stopped)
        {
            return foundation::Result<bool>(false);
        }

        const auto sizeResult = foundation::FileSystem::fileSize(m_path);

        if (!sizeResult)
        {
            return foundation::Result<bool>(sizeResult.error());
        }

        if (*sizeResult > m_readPosition)
        {
            if (m_stream.eof())
            {
                reopenFromPosition(m_readPosition);
            }

            const auto retry = tryReadLine(line);

            if (!retry)
            {
                return retry;
            }

            if (*retry)
            {
                return retry;
            }
        }

        std::this_thread::sleep_for(kTailPollInterval);
    }

    return foundation::Result<bool>(false);
}

} // namespace scope::source
