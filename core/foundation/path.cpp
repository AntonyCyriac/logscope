/**
 * @file path.cpp
 * @brief Path implementation.
 */

#include "path.hpp"

#include <filesystem>

namespace scope::foundation
{

Path::Path() noexcept = default;

Path::Path(std::string_view value)
    : m_path(value)
{
}

const std::string& Path::string() const noexcept
{
    return m_path;
}

bool Path::isEmpty() const noexcept
{
    return m_path.empty();
}

bool Path::isAbsolute() const
{
    return std::filesystem::path(m_path).is_absolute();
}

Path Path::parent() const
{
    return Path(std::filesystem::path(m_path).parent_path().string());
}

Path Path::filename() const
{
    return Path(std::filesystem::path(m_path).filename().string());
}

Path Path::extension() const
{
    return Path(std::filesystem::path(m_path).extension().string());
}

Path Path::append(std::string_view other) const
{
    std::filesystem::path combined(m_path);

    combined /= std::filesystem::path(other);

    return Path(combined.string());
}

bool operator==(const Path& lhs, const Path& rhs) noexcept
{
    return lhs.string() == rhs.string();
}

bool operator!=(const Path& lhs, const Path& rhs) noexcept
{
    return !(lhs == rhs);
}

} // namespace scope::foundation
