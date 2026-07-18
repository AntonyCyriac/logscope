/**
 * @file filesystem.cpp
 * @brief FileSystem implementation.
 */

#include "filesystem.hpp"

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

} // namespace scope::foundation
