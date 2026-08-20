/**
 * @file source_discovery.cpp
 * @brief Source discovery implementation.
 */

#include "source_discovery.hpp"

#include <algorithm>
#include <filesystem>

#include "foundation/error.hpp"
#include "foundation/filesystem.hpp"
#include "instance_grouper.hpp"
#include "text_sniff.hpp"

namespace scope::source
{

namespace
{

[[nodiscard]] std::string relativePathString(const foundation::Path& root, const foundation::Path& absolute)
{
    std::error_code error;
    const std::filesystem::path relative =
        std::filesystem::relative(std::filesystem::path(absolute.string()), std::filesystem::path(root.string()), error);

    if (error)
    {
        return absolute.string();
    }

    std::string normalized = relative.generic_string();

    if (normalized == ".")
    {
        return std::filesystem::path(absolute.string()).filename().string();
    }

    return normalized;
}

[[nodiscard]] foundation::Result<bool> appendCandidate(DiscoveryCensus& census, const foundation::Path& root,
                                                      const foundation::Path& absolutePath,
                                                      const DiscoveryOptions& options)
{
    if (census.candidatesFound >= options.maxFiles)
    {
        census.fileLimitHit = true;

        return foundation::Result<bool>(false);
    }

    DiscoveryEntry entry;
    entry.relativePath = relativePathString(root, absolutePath);
    entry.instanceKey = deriveInstanceKey(entry.relativePath);

    const auto sizeResult = foundation::FileSystem::fileSize(absolutePath);

    if (sizeResult)
    {
        entry.sizeBytes = *sizeResult;

        if (options.maxFileBytes > 0U && entry.sizeBytes > options.maxFileBytes)
        {
            entry.disposition = CandidateDisposition::Skipped;
            entry.skipReason = SkipReason::SizeLimit;
            census.entries.push_back(std::move(entry));
            ++census.candidatesFound;

            return foundation::Result<bool>(true);
        }
    }

    const auto sniffResult = sniffFileHead(absolutePath);

    if (!sniffResult)
    {
        entry.disposition = CandidateDisposition::Skipped;
        entry.skipReason = SkipReason::Unreadable;
        census.entries.push_back(std::move(entry));
        ++census.candidatesFound;

        return foundation::Result<bool>(true);
    }

    entry.headSniff = *sniffResult;

    if (*sniffResult == HeadSniffResult::BinaryCandidate)
    {
        entry.disposition = CandidateDisposition::Skipped;
        entry.skipReason = SkipReason::BinaryContent;
        census.entries.push_back(std::move(entry));
        ++census.candidatesFound;

        return foundation::Result<bool>(true);
    }

    entry.disposition = CandidateDisposition::Analyzed;
    entry.rotationGroupId = rotationGroupIdForPath(entry.relativePath);
    census.entries.push_back(std::move(entry));
    ++census.candidatesFound;

    return foundation::Result<bool>(true);
}

[[nodiscard]] foundation::Result<bool> walkDirectory(DiscoveryCensus& census, const foundation::Path& root,
                                                     const foundation::Path& directory, const std::uint32_t depth,
                                                     const DiscoveryOptions& options)
{
    if (depth > options.maxDepth)
    {
        census.depthLimitHit = true;

        return foundation::Result<bool>(true);
    }

    std::error_code error;
    const std::filesystem::path directoryPath(directory.string());

    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator(directoryPath, error))
    {
        if (error)
        {
            return foundation::Result<bool>(foundation::Error(foundation::ErrorCode::IOError, error.message()));
        }

        if (census.fileLimitHit)
        {
            break;
        }

        std::error_code entryError;

        if (entry.is_symlink(entryError))
        {
            if (entryError)
            {
                return foundation::Result<bool>(foundation::Error(foundation::ErrorCode::IOError, entryError.message()));
            }

            if (!std::filesystem::exists(entry.path(), entryError))
            {
                DiscoveryEntry symlinkEntry;
                symlinkEntry.relativePath = relativePathString(root, foundation::Path(entry.path().string()));
                symlinkEntry.disposition = CandidateDisposition::Skipped;
                symlinkEntry.skipReason = SkipReason::BrokenSymlink;
                symlinkEntry.instanceKey = deriveInstanceKey(symlinkEntry.relativePath);
                census.entries.push_back(std::move(symlinkEntry));
                ++census.candidatesFound;
                continue;
            }
        }

        if (entry.is_regular_file(entryError))
        {
            if (entryError)
            {
                return foundation::Result<bool>(foundation::Error(foundation::ErrorCode::IOError, entryError.message()));
            }

            const auto appendResult =
                appendCandidate(census, root, foundation::Path(entry.path().string()), options);

            if (!appendResult)
            {
                return appendResult;
            }

            continue;
        }

        if (options.recursive && entry.is_directory(entryError))
        {
            if (entryError)
            {
                return foundation::Result<bool>(foundation::Error(foundation::ErrorCode::IOError, entryError.message()));
            }

            const auto nestedResult = walkDirectory(census, root, foundation::Path(entry.path().string()), depth + 1U,
                                                  options);

            if (!nestedResult)
            {
                return nestedResult;
            }
        }
    }

    return foundation::Result<bool>(true);
}

void finalizeCensusMetadata(DiscoveryCensus& census, const std::vector<IngestStream>& streams)
{
    for (const IngestStream& stream : streams)
    {
        if (!stream.rotationGroupId.empty())
        {
            RotationGroup group;
            group.groupId = stream.rotationGroupId;

            for (const IngestFile& file : stream.orderedFiles)
            {
                group.orderedRelativePaths.push_back(file.relativePath);
            }

            census.rotationGroups.push_back(std::move(group));
        }
    }

    std::vector<InstanceSummary> summaries;

    for (const DiscoveryEntry& entry : census.entries)
    {
        auto summaryIt = std::find_if(summaries.begin(), summaries.end(), [&](const InstanceSummary& summary) {
            return summary.key == entry.instanceKey;
        });

        if (summaryIt == summaries.end())
        {
            InstanceSummary summary;
            summary.key = entry.instanceKey;
            summary.filesDiscovered = 1U;

            if (entry.disposition == CandidateDisposition::Analyzed)
            {
                summary.filesAnalyzed = 1U;
            }

            summaries.push_back(std::move(summary));
        }
        else
        {
            ++summaryIt->filesDiscovered;

            if (entry.disposition == CandidateDisposition::Analyzed)
            {
                ++summaryIt->filesAnalyzed;
            }
        }
    }

    census.instances = std::move(summaries);
    census.analyzedCount = 0U;

    for (const DiscoveryEntry& entry : census.entries)
    {
        if (entry.disposition == CandidateDisposition::Analyzed)
        {
            ++census.analyzedCount;
        }
    }
}

} // namespace

foundation::Result<DiscoveryScanResult> discoverSource(const foundation::Path& root, const DiscoveryOptions& options)
{
    DiscoveryScanResult result;
    result.census.root = root;

    const auto isFileResult = foundation::FileSystem::isFile(root);

    if (!isFileResult)
    {
        return foundation::Result<DiscoveryScanResult>(isFileResult.error());
    }

    if (*isFileResult)
    {
        const auto appendResult = appendCandidate(result.census, root, root, options);

        if (!appendResult)
        {
            return foundation::Result<DiscoveryScanResult>(appendResult.error());
        }
    }
    else
    {
        const auto walkResult = walkDirectory(result.census, root, root, 0U, options);

        if (!walkResult)
        {
            return foundation::Result<DiscoveryScanResult>(walkResult.error());
        }
    }

    std::vector<IngestFile> textCandidates;

    for (const DiscoveryEntry& entry : result.census.entries)
    {
        if (entry.disposition != CandidateDisposition::Analyzed)
        {
            continue;
        }

        IngestFile file;
        file.relativePath = entry.relativePath;
        file.absolutePath = root;

        if (root.string() != entry.relativePath)
        {
            file.absolutePath = foundation::Path((std::filesystem::path(root.string()) / entry.relativePath).string());
        }

        file.instanceKey = entry.instanceKey;
        textCandidates.push_back(std::move(file));
    }

    result.ingestStreams = buildIngestStreams(root, textCandidates);
    finalizeCensusMetadata(result.census, result.ingestStreams);

    return foundation::Result<DiscoveryScanResult>(std::move(result));
}

} // namespace scope::source
