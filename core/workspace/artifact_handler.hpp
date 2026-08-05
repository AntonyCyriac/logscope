/**
 * @file artifact_handler.hpp
 * @brief Artifact type handlers (Story 1 / v2.3.0).
 */

#pragma once

#include <string_view>

#include "artifact_record.hpp"
#include "foundation/path.hpp"
#include "foundation/result.hpp"

namespace scope::workspace
{

/**
 * @brief Storage context for artifact import.
 */
struct ArtifactIngestContext
{
    foundation::Path investigationRoot;
    foundation::Path artifactDirectory;
    std::string artifactId;
};

/**
 * @brief Import request for a new artifact.
 */
struct ArtifactIngestRequest
{
    std::string type;
    std::string name;
    foundation::Path sourceFile;
    std::string noteBody;
    std::string role;
    ArtifactSource source;
};

/**
 * @brief Returns true when artifact type may be opened into an analyze/investigate session.
 */
[[nodiscard]] bool artifactTypeSupportsSessionOpen(std::string_view type) noexcept;

/**
 * @brief Handler for a single artifact type string.
 */
class IArtifactHandler
{
  public:
    virtual ~IArtifactHandler() = default;

    [[nodiscard]] virtual std::string_view type() const = 0;

    [[nodiscard]] virtual foundation::Result<ArtifactRecord> ingest(const ArtifactIngestContext& context,
                                                                      const ArtifactIngestRequest& request) const = 0;

    [[nodiscard]] virtual foundation::Result<foundation::Path> resolveDataPath(
        const foundation::Path& investigationRoot, const ArtifactRecord& artifact) const = 0;
};

/**
 * @brief Returns the handler for a type string, or nullptr if unknown.
 */
[[nodiscard]] const IArtifactHandler* findArtifactHandler(std::string_view type) noexcept;

} // namespace scope::workspace
