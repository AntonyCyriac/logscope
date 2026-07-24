/**
 * @file workspace_paths.cpp
 */

#include "workspace_paths.hpp"

#include <cstdlib>
#include <filesystem>

namespace scope::storage
{

foundation::Path defaultIndexDirectory() noexcept
{
#if defined(_WIN32)
    if (const char* localAppData = std::getenv("LOCALAPPDATA"); localAppData != nullptr)
    {
        return foundation::Path((std::filesystem::path(localAppData) / "logscope" / "indexes").string());
    }
#endif

    if (const char* home = std::getenv("HOME"); home != nullptr)
    {
        return foundation::Path((std::filesystem::path(home) / ".logscope" / "indexes").string());
    }

    if (const char* userProfile = std::getenv("USERPROFILE"); userProfile != nullptr)
    {
        return foundation::Path((std::filesystem::path(userProfile) / ".logscope" / "indexes").string());
    }

    return foundation::Path(".logscope/indexes");
}

} // namespace scope::storage
