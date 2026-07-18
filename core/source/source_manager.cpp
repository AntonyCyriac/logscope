/**
 * @file source_manager.cpp
 * @brief SourceManager implementation.
 */

#include "source_manager.hpp"

#include <vector>

#include "composite_log_source.hpp"
#include "file_log_source.hpp"
#include "foundation/error.hpp"
#include "foundation/filesystem.hpp"
#include "foundation/string.hpp"
#include "log_macros.hpp"
#include "stdin_log_source.hpp"

namespace scope::source
{

namespace
{

bool isStdinPath(const foundation::Path& path) noexcept
{
    return path.string() == "-";
}

bool isLogFile(const foundation::Path& path)
{
    return foundation::endsWith(foundation::toLower(path.string()), ".log");
}

foundation::Result<std::vector<foundation::Path>> listLogFilesInDirectory(const foundation::Path& directory)
{
    const auto filesResult = foundation::FileSystem::listRegularFiles(directory);

    if (!filesResult)
    {
        return foundation::Result<std::vector<foundation::Path>>(filesResult.error());
    }

    std::vector<foundation::Path> logFiles;

    for (const foundation::Path& file : *filesResult)
    {
        if (isLogFile(file))
        {
            logFiles.push_back(file);
        }
    }

    return foundation::Result<std::vector<foundation::Path>>(std::move(logFiles));
}

} // namespace

foundation::Result<bool> SourceManager::validate(const foundation::Path& path) const
{
    SCOPE_LOG_DEBUG("source", "Validating source: " + path.string());

    if (isStdinPath(path))
    {
        SCOPE_LOG_DEBUG("source", "Source validated: stdin");

        return foundation::Result<bool>(true);
    }

    const auto existsResult = foundation::FileSystem::exists(path);

    if (!existsResult)
    {
        return foundation::Result<bool>(existsResult.error());
    }

    if (!*existsResult)
    {
        SCOPE_LOG_ERROR("source", "Source not found: " + path.string());

        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Log source not found."));
    }

    const auto isFileResult = foundation::FileSystem::isFile(path);

    if (!isFileResult)
    {
        return foundation::Result<bool>(isFileResult.error());
    }

    if (*isFileResult)
    {
        SCOPE_LOG_DEBUG("source", "Source validated: " + path.string());

        return foundation::Result<bool>(true);
    }

    const auto isDirectoryResult = foundation::FileSystem::isDirectory(path);

    if (!isDirectoryResult)
    {
        return foundation::Result<bool>(isDirectoryResult.error());
    }

    if (!*isDirectoryResult)
    {
        SCOPE_LOG_ERROR("source", "Unsupported source type: " + path.string());

        return foundation::Result<bool>(foundation::Error(
            foundation::ErrorCode::InvalidArgument,
            "Log source must be a file, directory, or stdin ('-')."));
    }

    const auto logFilesResult = listLogFilesInDirectory(path);

    if (!logFilesResult)
    {
        return foundation::Result<bool>(logFilesResult.error());
    }

    if (logFilesResult->empty())
    {
        SCOPE_LOG_ERROR("source", "No log files found in directory: " + path.string());

        return foundation::Result<bool>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "No log files found in directory."));
    }

    SCOPE_LOG_DEBUG("source",
                    "Source validated directory: " + path.string() + " (" +
                        std::to_string(logFilesResult->size()) + " log file(s))");

    return foundation::Result<bool>(true);
}

foundation::Result<SourceDataset> SourceManager::open(const foundation::Path& path) const
{
    SCOPE_LOG_INFO("source", "Opening source: " + path.string());

    const auto validationResult = validate(path);

    if (!validationResult)
    {
        return foundation::Result<SourceDataset>(validationResult.error());
    }

    if (isStdinPath(path))
    {
        auto sourceResult = StdinLogSource::open();

        if (!sourceResult)
        {
            return foundation::Result<SourceDataset>(sourceResult.error());
        }

        return foundation::Result<SourceDataset>(SourceDataset(std::move(*sourceResult)));
    }

    const auto isFileResult = foundation::FileSystem::isFile(path);

    if (!isFileResult)
    {
        return foundation::Result<SourceDataset>(isFileResult.error());
    }

    if (*isFileResult)
    {
        auto sourceResult = FileLogSource::open(path);

        if (!sourceResult)
        {
            return foundation::Result<SourceDataset>(sourceResult.error());
        }

        return foundation::Result<SourceDataset>(SourceDataset(std::move(*sourceResult)));
    }

    const auto logFilesResult = listLogFilesInDirectory(path);

    if (!logFilesResult)
    {
        return foundation::Result<SourceDataset>(logFilesResult.error());
    }

    std::vector<std::unique_ptr<LogSource>> sources;

    for (const foundation::Path& logFile : *logFilesResult)
    {
        auto sourceResult = FileLogSource::open(logFile);

        if (!sourceResult)
        {
            return foundation::Result<SourceDataset>(sourceResult.error());
        }

        sources.push_back(std::move(*sourceResult));
    }

    auto compositeResult = CompositeLogSource::create(path, std::move(sources));

    if (!compositeResult)
    {
        return foundation::Result<SourceDataset>(compositeResult.error());
    }

    return foundation::Result<SourceDataset>(SourceDataset(std::move(*compositeResult)));
}

} // namespace scope::source
