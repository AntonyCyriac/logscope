/**
 * @file source_manager.cpp
 * @brief SourceManager implementation.
 */

#include "source_manager.hpp"

#include "attributed_ingest_source.hpp"
#include "file_log_source.hpp"
#include "foundation/error.hpp"
#include "foundation/filesystem.hpp"
#include "log_macros.hpp"
#include "source_discovery.hpp"
#include "stdin_log_source.hpp"
#include "tailing_file_log_source.hpp"

namespace scope::source
{

namespace
{

bool isStdinPath(const foundation::Path& path) noexcept
{
    return path.string() == "-";
}

[[nodiscard]] bool censusHasAnalyzableContent(const DiscoveryCensus& census) noexcept
{
    return census.analyzedCount > 0U;
}

[[nodiscard]] bool censusIsComplete(const DiscoveryCensus& census) noexcept
{
    if (census.depthLimitHit || census.fileLimitHit)
    {
        return false;
    }

    for (const DiscoveryEntry& entry : census.entries)
    {
        if (entry.disposition == CandidateDisposition::Skipped)
        {
            return false;
        }
    }

    return true;
}

} // namespace

foundation::Result<bool> SourceManager::validate(const foundation::Path& path,
                                                 const DiscoveryOptions& options) const
{
    SCOPE_LOG_DEBUG("source", "Validating source: " + path.string());

    if (isStdinPath(path))
    {
        return foundation::Result<bool>(true);
    }

    if (isArchivePath(path))
    {
        return foundation::Result<bool>(foundation::Error(
            foundation::ErrorCode::InvalidArgument,
            "Archive sources must be extracted before analysis. Extract the archive and pass the directory path."));
    }

    const auto existsResult = foundation::FileSystem::exists(path);

    if (!existsResult)
    {
        return foundation::Result<bool>(existsResult.error());
    }

    if (!*existsResult)
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Log source not found."));
    }

    const auto discoverResult = discoverSource(path, options);

    if (!discoverResult)
    {
        return foundation::Result<bool>(discoverResult.error());
    }

    if (discoverResult->census.candidatesFound == 0U)
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "No files found in directory."));
    }

    if (!censusHasAnalyzableContent(discoverResult->census))
    {
        return foundation::Result<bool>(foundation::Error(
            foundation::ErrorCode::Indeterminate,
            "Candidates were found but none could be ingested. See discovery census for skip reasons."));
    }

    return foundation::Result<bool>(true);
}

foundation::Result<SourceDataset> SourceManager::open(const foundation::Path& path) const
{
    return open(path, OpenOptions{});
}

foundation::Result<SourceDataset> SourceManager::open(const foundation::Path& path, const OpenOptions options) const
{
    SCOPE_LOG_INFO("source", "Opening source: " + path.string());

    if (isStdinPath(path))
    {
        auto sourceResult = StdinLogSource::open();

        if (!sourceResult)
        {
            return foundation::Result<SourceDataset>(sourceResult.error());
        }

        return foundation::Result<SourceDataset>(SourceDataset(std::move(*sourceResult)));
    }

    if (isArchivePath(path))
    {
        return foundation::Result<SourceDataset>(foundation::Error(
            foundation::ErrorCode::InvalidArgument,
            "Archive sources must be extracted before analysis. Extract the archive and pass the directory path."));
    }

    const auto validationResult = validate(path, options.discovery);

    if (!validationResult)
    {
        return foundation::Result<SourceDataset>(validationResult.error());
    }

    const auto isFileResult = foundation::FileSystem::isFile(path);

    if (!isFileResult)
    {
        return foundation::Result<SourceDataset>(isFileResult.error());
    }

    if (*isFileResult && options.follow)
    {
        auto sourceResult = TailingFileLogSource::open(path, true);

        if (!sourceResult)
        {
            return foundation::Result<SourceDataset>(sourceResult.error());
        }

        return foundation::Result<SourceDataset>(SourceDataset(std::move(*sourceResult)));
    }

    const auto discoverResult = discoverSource(path, options.discovery);

    if (!discoverResult)
    {
        return foundation::Result<SourceDataset>(discoverResult.error());
    }

    if (!censusHasAnalyzableContent(discoverResult->census))
    {
        return foundation::Result<SourceDataset>(foundation::Error(
            foundation::ErrorCode::Indeterminate,
            "Candidates were found but none could be ingested. See discovery census for skip reasons."));
    }

    auto attributedResult =
        AttributedIngestSource::create(path, std::move(discoverResult->ingestStreams));

    if (!attributedResult)
    {
        return foundation::Result<SourceDataset>(attributedResult.error());
    }

    SourceDataset dataset(std::move(*attributedResult), std::move(discoverResult->census));
    dataset.analysisAccounting().complete = censusIsComplete(dataset.discoveryCensus());

    return foundation::Result<SourceDataset>(std::move(dataset));
}

} // namespace scope::source
