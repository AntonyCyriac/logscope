/**
 * @file crash_analyzer.hpp
 * @brief Pluggable crash analyzers for investigation artifacts (Story 4 / v2.6.0).
 */

#pragma once

#include <string_view>

#include "artifact_record.hpp"
#include "crash_report.hpp"
#include "foundation/path.hpp"
#include "foundation/result.hpp"

namespace scope::workspace
{

/**
 * @brief Analyzes one artifact type into a CrashReport projection.
 */
class IArtifactCrashAnalyzer
{
  public:
    virtual ~IArtifactCrashAnalyzer() = default;

    [[nodiscard]] virtual std::string_view artifactType() const noexcept = 0;

    [[nodiscard]] virtual bool supports(const ArtifactRecord& artifact) const = 0;

    [[nodiscard]] virtual bool canAnalyze(const ArtifactRecord& artifact,
                                            const foundation::Path& dataPath) const = 0;

    [[nodiscard]] virtual foundation::Result<CrashReport> analyze(
        const ArtifactRecord& artifact, const foundation::Path& dataPath,
        const CrashAnalysisContext& context) const = 0;
};

/**
 * @brief Returns the crash analyzer for an artifact type, or nullptr if none.
 */
[[nodiscard]] const IArtifactCrashAnalyzer* findCrashAnalyzer(std::string_view type) noexcept;

/**
 * @brief Whether the artifact type can produce a crash report.
 */
[[nodiscard]] bool isCrashAnalyzableArtifactType(std::string_view type) noexcept;

} // namespace scope::workspace
