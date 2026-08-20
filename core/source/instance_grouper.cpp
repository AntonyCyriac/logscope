/**
 * @file instance_grouper.cpp
 * @brief Instance key derivation.
 */

#include "instance_grouper.hpp"

#include <cctype>

namespace scope::source
{

namespace
{

[[nodiscard]] bool isGenericSegment(const std::string_view segment) noexcept
{
    return segment == "logs" || segment == "log" || segment == "var" || segment == "tmp" || segment == "data";
}

[[nodiscard]] bool matchesInstanceSegment(const std::string_view segment) noexcept
{
    if (segment.size() < 4U)
    {
        return false;
    }

    const auto prefixMatch = [&](const std::string_view prefix) noexcept {
        if (segment.size() < prefix.size())
        {
            return false;
        }

        for (std::size_t index = 0U; index < prefix.size(); ++index)
        {
            if (std::tolower(static_cast<unsigned char>(segment[index])) !=
                static_cast<unsigned char>(prefix[index]))
            {
                return false;
            }
        }

        return segment.size() == prefix.size() || segment[prefix.size()] == '-';
    };

    return prefixMatch("instance") || prefixMatch("pod") || prefixMatch("replica") || prefixMatch("node");
}

[[nodiscard]] std::string firstSegment(std::string_view relativePath)
{
    const std::size_t slash = relativePath.find('/');

    if (slash == std::string_view::npos)
    {
        return std::string(relativePath);
    }

    return std::string(relativePath.substr(0U, slash));
}

} // namespace

std::string deriveInstanceKey(const std::string_view relativePath) noexcept
{
    std::string_view remaining = relativePath;

    while (!remaining.empty())
    {
        const std::size_t slash = remaining.find('/');

        const std::string_view segment =
            slash == std::string_view::npos ? remaining : remaining.substr(0U, slash);

        if (matchesInstanceSegment(segment))
        {
            return std::string(segment);
        }

        if (slash == std::string_view::npos)
        {
            break;
        }

        remaining.remove_prefix(slash + 1U);
    }

    const std::string parent = firstSegment(relativePath);

    if (!parent.empty() && parent != relativePath && !isGenericSegment(parent))
    {
        return parent;
    }

    return "default";
}

} // namespace scope::source
