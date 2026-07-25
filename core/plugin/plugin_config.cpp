/**
 * @file plugin_config.cpp
 */

#include "plugin_config.hpp"

#include <cstdlib>

#include "foundation/error.hpp"
#include "foundation/filesystem.hpp"
#include "foundation/string.hpp"

namespace scope::plugin
{

namespace
{

#if defined(_WIN32)
constexpr char kPathSeparator = ';';
#else
constexpr char kPathSeparator = ':';
#endif

void appendUniquePath(std::vector<foundation::Path>& paths, const foundation::Path& path)
{
    if (path.string().empty())
    {
        return;
    }

    for (const foundation::Path& existing : paths)
    {
        if (existing == path)
        {
            return;
        }
    }

    paths.push_back(path);
}

void appendPathsFromDelimitedList(std::vector<foundation::Path>& paths, const std::string& value)
{
    std::string current;

    for (const char character : value)
    {
        if (character == kPathSeparator)
        {
            appendUniquePath(paths, foundation::Path(current));

            current.clear();

            continue;
        }

        current.push_back(character);
    }

    appendUniquePath(paths, foundation::Path(current));
}

} // namespace

PluginConfig resolvePluginConfig(const runtime::Configuration& configuration) noexcept
{
    PluginConfig config;

    if (configuration.has("plugins.enabled"))
    {
        const auto enabledResult = configuration.get("plugins.enabled");

        if (enabledResult)
        {
            const std::string normalized = foundation::toLower(foundation::trim(*enabledResult));
            config.enabled =
                normalized == "true" || normalized == "1" || normalized == "yes" || normalized == "on";
        }
    }

    if (configuration.has("plugins.paths"))
    {
        const auto pathsResult = configuration.get("plugins.paths");

        if (pathsResult)
        {
            appendPathsFromDelimitedList(config.paths, *pathsResult);
        }
    }

    return config;
}

std::vector<foundation::Path> mergePluginPaths(const PluginConfig& config)
{
    std::vector<foundation::Path> paths = config.paths;

    if (const char* envPath = std::getenv("LOGSCOPE_PLUGIN_PATH"))
    {
        appendPathsFromDelimitedList(paths, envPath);
    }

    return paths;
}

foundation::Result<bool> validatePluginConfiguration(const runtime::Configuration& configuration) noexcept
{
    if (!configuration.has("plugins.paths"))
    {
        return foundation::Result<bool>(true);
    }

    const auto pathsResult = configuration.get("plugins.paths");

    if (!pathsResult)
    {
        return foundation::Result<bool>(pathsResult.error());
    }

    PluginConfig config = resolvePluginConfig(configuration);
    const std::vector<foundation::Path> paths = mergePluginPaths(config);

    for (const foundation::Path& path : paths)
    {
        const auto existsResult = foundation::FileSystem::exists(path);

        if (!existsResult || !*existsResult)
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::InvalidArgument, "Plugin path does not exist: " + path.string()));
        }
    }

    return foundation::Result<bool>(true);
}

} // namespace scope::plugin
