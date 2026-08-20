/**
 * @file discovery_census.cpp
 * @brief Discovery census helpers.
 */

#include "discovery_census.hpp"

#include "foundation/string.hpp"

namespace scope::source
{

const char* skipReasonName(const SkipReason reason) noexcept
{
    switch (reason)
    {
    case SkipReason::BinaryContent:
        return "BINARY_CONTENT";
    case SkipReason::Unreadable:
        return "UNREADABLE";
    case SkipReason::BrokenSymlink:
        return "BROKEN_SYMLINK";
    case SkipReason::DepthLimit:
        return "DEPTH_LIMIT";
    case SkipReason::FileLimit:
        return "FILE_LIMIT";
    case SkipReason::SizeLimit:
        return "SIZE_LIMIT";
    case SkipReason::ExcludedByConfig:
        return "EXCLUDED_BY_CONFIG";
    }

    return "UNKNOWN";
}

const char* candidateDispositionName(const CandidateDisposition disposition) noexcept
{
    switch (disposition)
    {
    case CandidateDisposition::Analyzed:
        return "ANALYZED";
    case CandidateDisposition::Skipped:
        return "SKIPPED";
    case CandidateDisposition::Unsupported:
        return "UNSUPPORTED";
    case CandidateDisposition::Failed:
        return "FAILED";
    }

    return "UNKNOWN";
}

bool isArchivePath(const foundation::Path& path) noexcept
{
    const std::string lower = foundation::toLower(path.string());

    if (foundation::endsWith(lower, ".tar.gz") || foundation::endsWith(lower, ".tgz"))
    {
        return true;
    }

    if (foundation::endsWith(lower, ".tar") || foundation::endsWith(lower, ".zip"))
    {
        return true;
    }

    return false;
}

} // namespace scope::source
