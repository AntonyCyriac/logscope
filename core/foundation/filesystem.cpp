/**
 * @file filesystem.cpp
 * @brief FileSystem implementation.
 */

#include "filesystem.hpp"

#include <algorithm>
#include <filesystem>
#include <system_error>

#include "error.hpp"

namespace scope::foundation
{

Result<bool> FileSystem::exists(const Path& path)
{
    std::error_code error;

    const bool result = std::filesystem::exists(std::filesystem::path(path.string()), error);

    if (error)
    {
        return Result<bool>(Error(ErrorCode::IOError, error.message()));
    }

    return Result<bool>(result);
}

Result<bool> FileSystem::isFile(const Path& path)
{
    std::error_code error;

    const bool result =
        std::filesystem::is_regular_file(std::filesystem::path(path.string()), error);

    if (error)
    {
        return Result<bool>(Error(ErrorCode::IOError, error.message()));
    }

    return Result<bool>(result);
}

Result<bool> FileSystem::isDirectory(const Path& path)
{
    std::error_code error;

    const bool result = std::filesystem::is_directory(std::filesystem::path(path.string()), error);

    if (error)
    {
        return Result<bool>(Error(ErrorCode::IOError, error.message()));
    }

    return Result<bool>(result);
}

Result<std::uint64_t> FileSystem::fileSize(const Path& path)
{
    std::error_code error;

    const auto size = std::filesystem::file_size(std::filesystem::path(path.string()), error);

    if (error)
    {
        return Result<std::uint64_t>(Error(ErrorCode::IOError, error.message()));
    }

    return Result<std::uint64_t>(size);
}

Result<std::vector<Path>> FileSystem::listRegularFiles(const Path& directory)
{
    std::error_code error;
    const std::filesystem::path directoryPath(directory.string());

    if (!std::filesystem::is_directory(directoryPath, error))
    {
        if (error)
        {
            return Result<std::vector<Path>>(Error(ErrorCode::IOError, error.message()));
        }

        return Result<std::vector<Path>>(
            Error(ErrorCode::InvalidArgument, "Path is not a directory."));
    }

    std::vector<Path> files;

    for (const auto& entry : std::filesystem::directory_iterator(directoryPath, error))
    {
        if (error)
        {
            return Result<std::vector<Path>>(Error(ErrorCode::IOError, error.message()));
        }

        if (entry.is_regular_file(error))
        {
            if (error)
            {
                return Result<std::vector<Path>>(Error(ErrorCode::IOError, error.message()));
            }

            files.emplace_back(entry.path().string());
        }
    }

    std::sort(files.begin(), files.end(), [](const Path& left, const Path& right) {
        return left.string() < right.string();
    });

    return Result<std::vector<Path>>(std::move(files));
}

} // namespace scope::foundation
