/**
 * @file source_manager.cpp
 * @brief SourceManager implementation.
 */

#include "source_manager.hpp"

#include "file_log_source.hpp"
#include "foundation/error.hpp"
#include "foundation/filesystem.hpp"
#include "log_macros.hpp"

namespace scope::source
{

foundation::Result<bool> SourceManager::validate(const foundation::Path& path) const
{
    SCOPE_LOG_DEBUG("source", "Validating source: " + path.string());

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

    if (!*isFileResult)
    {
        SCOPE_LOG_ERROR("source", "Source is not a file: " + path.string());

        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Log source is not a file."));
    }

    SCOPE_LOG_DEBUG("source", "Source validated: " + path.string());

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

    auto sourceResult = FileLogSource::open(path);

    if (!sourceResult)
    {
        return foundation::Result<SourceDataset>(sourceResult.error());
    }

    return foundation::Result<SourceDataset>(SourceDataset(std::move(*sourceResult)));
}

} // namespace scope::source
