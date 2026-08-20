/**
 * @file rotation_grouper.cpp
 * @brief Rotation stream grouping.
 */

#include "rotation_grouper.hpp"

#include <algorithm>
#include <cctype>

#include "foundation/string.hpp"

namespace scope::source
{

namespace
{

[[nodiscard]] std::optional<std::pair<std::string, std::optional<std::uint64_t>>>
parseRotationSuffix(std::string_view fileName) noexcept
{
    const std::size_t dotLog = foundation::toLower(std::string(fileName)).find(".log");

    if (dotLog == std::string_view::npos)
    {
        return std::nullopt;
    }

    const std::string base = std::string(fileName.substr(0U, dotLog + 4U));

    if (fileName.size() == dotLog + 4U)
    {
        return std::make_pair(base, std::optional<std::uint64_t>{});
    }

    const std::string_view suffix = fileName.substr(dotLog + 4U);

    if (!suffix.empty() && suffix.front() == '.')
    {
        std::uint64_t value = 0U;
        bool digits = !suffix.substr(1U).empty();

        for (char character : suffix.substr(1U))
        {
            if (!std::isdigit(static_cast<unsigned char>(character)))
            {
                digits = false;
                break;
            }

            value = value * 10U + static_cast<std::uint64_t>(character - '0');
        }

        if (digits)
        {
            return std::make_pair(base, value);
        }
    }

    return std::make_pair(base, std::optional<std::uint64_t>{});
}

[[nodiscard]] std::string fileNameFromRelative(std::string_view relativePath)
{
    const std::size_t slash = relativePath.find_last_of("/\\");

    if (slash == std::string_view::npos)
    {
        return std::string(relativePath);
    }

    return std::string(relativePath.substr(slash + 1U));
}

} // namespace

std::optional<std::string> rotationGroupIdForPath(const std::string_view relativePath) noexcept
{
    const auto parsed = parseRotationSuffix(fileNameFromRelative(relativePath));

    if (!parsed.has_value())
    {
        return std::nullopt;
    }

    return parsed->first;
}

std::vector<IngestStream> buildIngestStreams(const foundation::Path& root,
                                             const std::vector<IngestFile>& textCandidates)
{
    std::vector<IngestStream> streams;
    std::vector<IngestFile> singletons;

    struct GroupBuilder
    {
        std::string groupId;
        std::vector<IngestFile> files;
    };

    std::vector<GroupBuilder> groups;

    for (const IngestFile& candidate : textCandidates)
    {
        const auto groupId = rotationGroupIdForPath(candidate.relativePath);

        if (!groupId.has_value())
        {
            singletons.push_back(candidate);
            continue;
        }

        auto groupIt = std::find_if(groups.begin(), groups.end(), [&](const GroupBuilder& group) {
            return group.groupId == *groupId;
        });

        if (groupIt == groups.end())
        {
            groups.push_back(GroupBuilder{*groupId, {candidate}});
        }
        else
        {
            groupIt->files.push_back(candidate);
        }
    }

    for (GroupBuilder& group : groups)
    {
        std::sort(group.files.begin(), group.files.end(), [](const IngestFile& left, const IngestFile& right) {
            const auto leftParsed = parseRotationSuffix(fileNameFromRelative(left.relativePath));
            const auto rightParsed = parseRotationSuffix(fileNameFromRelative(right.relativePath));

            const std::uint64_t leftOrder = leftParsed && leftParsed->second.has_value() ? *leftParsed->second : 999999U;
            const std::uint64_t rightOrder =
                rightParsed && rightParsed->second.has_value() ? *rightParsed->second : 999999U;

            if (leftOrder != rightOrder)
            {
                return leftOrder < rightOrder;
            }

            return left.relativePath < right.relativePath;
        });

        IngestStream stream;
        stream.streamId = group.groupId;
        stream.rotationGroupId = group.groupId;
        stream.instanceKey = group.files.front().instanceKey;
        stream.orderedFiles = std::move(group.files);
        streams.push_back(std::move(stream));
    }

    for (IngestFile& singleton : singletons)
    {
        IngestStream stream;
        stream.streamId = singleton.relativePath;
        stream.instanceKey = singleton.instanceKey;
        stream.orderedFiles.push_back(std::move(singleton));
        streams.push_back(std::move(stream));
    }

    std::sort(streams.begin(), streams.end(), [](const IngestStream& left, const IngestStream& right) {
        return left.streamId < right.streamId;
    });

    (void)root;

    return streams;
}

} // namespace scope::source
