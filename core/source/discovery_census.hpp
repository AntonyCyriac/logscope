/**
 * @file discovery_census.hpp
 * @brief Discovery census and ingestion accounting (H0 Wave 3).
 */

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "foundation/path.hpp"

namespace scope::source
{

enum class CandidateDisposition
{
    Analyzed,
    Skipped,
    Unsupported,
    Failed
};

enum class SkipReason
{
    BinaryContent,
    Unreadable,
    BrokenSymlink,
    DepthLimit,
    FileLimit,
    SizeLimit,
    ExcludedByConfig
};

enum class HeadSniffResult
{
    TextCandidate,
    BinaryCandidate
};

struct DiscoveryEntry
{
    std::string relativePath;
    std::uint64_t sizeBytes{0U};
    CandidateDisposition disposition{CandidateDisposition::Skipped};
    std::optional<SkipReason> skipReason;
    std::string instanceKey;
    std::optional<std::string> rotationGroupId;
    HeadSniffResult headSniff{HeadSniffResult::TextCandidate};
};

struct RotationGroup
{
    std::string groupId;
    std::vector<std::string> orderedRelativePaths;
};

struct InstanceSummary
{
    std::string key;
    std::uint32_t filesDiscovered{0U};
    std::uint32_t filesAnalyzed{0U};
    std::uint64_t linesAnalyzed{0U};
};

struct DiscoveryCensus
{
    foundation::Path root;
    std::uint32_t candidatesFound{0U};
    std::uint32_t analyzedCount{0U};
    std::vector<DiscoveryEntry> entries;
    std::vector<RotationGroup> rotationGroups;
    std::vector<InstanceSummary> instances;
    bool depthLimitHit{false};
    bool fileLimitHit{false};
};

struct PerFileAnalysis
{
    std::string relativePath;
    std::uint64_t linesIngested{0U};
    std::uint64_t linesAnalyzed{0U};
    std::string timestampDialect{"n/a"};
};

struct IngestionWarning
{
    std::string code;
    std::string message;
};

struct AnalysisAccounting
{
    std::uint64_t streamLineCount{0U};
    std::uint64_t analyzedLineCount{0U};
    bool complete{true};
    std::vector<PerFileAnalysis> perFile;
    std::vector<IngestionWarning> warnings;
};

struct LineAttribution
{
    foundation::Path sourceFile;
    std::string sourceFileRelative;
    std::uint64_t fileLineNumber{0U};
    std::uint64_t streamLineNumber{0U};
};

struct DiscoveryOptions
{
    bool recursive{true};
    std::uint32_t maxDepth{16U};
    std::uint32_t maxFiles{10000U};
    std::uint64_t maxFileBytes{0U};
};

[[nodiscard]] const char* skipReasonName(SkipReason reason) noexcept;

[[nodiscard]] const char* candidateDispositionName(CandidateDisposition disposition) noexcept;

[[nodiscard]] bool isArchivePath(const foundation::Path& path) noexcept;

} // namespace scope::source
