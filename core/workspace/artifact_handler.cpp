/**
 * @file artifact_handler.cpp
 */

#include "artifact_handler.hpp"

#include "foundation/clock.hpp"
#include "foundation/uuid.hpp"

#include <filesystem>
#include <fstream>

namespace scope::workspace
{

namespace
{

std::string currentTimestampIso()
{
    return foundation::Clock::now().toString();
}

foundation::Result<bool> copyFileTo(const foundation::Path& source, const foundation::Path& destination)
{
    std::error_code errorCode;
    std::filesystem::copy_file(source.string(), destination.string(),
                             std::filesystem::copy_options::overwrite_existing, errorCode);

    if (errorCode)
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::IOError, "Failed to copy artifact file."));
    }

    return foundation::Result<bool>(true);
}

class LogArtifactHandler final : public IArtifactHandler
{
  public:
    [[nodiscard]] std::string_view type() const override
    {
        return "log";
    }

    [[nodiscard]] foundation::Result<ArtifactRecord> ingest(const ArtifactIngestContext& context,
                                                            const ArtifactIngestRequest& request) const override
    {
        if (request.sourceFile.string().empty())
        {
            return foundation::Result<ArtifactRecord>(foundation::Error(
                foundation::ErrorCode::InvalidArgument, "Log artifact requires a source file."));
        }

        const foundation::Path dataPath = foundation::Path(context.artifactDirectory.string() + "/data");
        const auto copyResult = copyFileTo(request.sourceFile, dataPath);

        if (!copyResult)
        {
            return foundation::Result<ArtifactRecord>(copyResult.error());
        }

        ArtifactRecord record;
        record.id = context.artifactId.empty() ? foundation::Uuid::generate().toString() : context.artifactId;
        record.type = std::string(type());
        record.name = request.name.empty() ? request.sourceFile.filename().string() : request.name;
        record.relativePath = "artifacts/" + record.id + "/data";
        record.importedAt = currentTimestampIso();
        record.source = request.source;
        record.status = "ready";

        return foundation::Result<ArtifactRecord>(std::move(record));
    }

    [[nodiscard]] foundation::Result<foundation::Path> resolveDataPath(
        const foundation::Path& investigationRoot, const ArtifactRecord& artifact) const override
    {
        const foundation::Path path = foundation::Path(investigationRoot.string() + "/" + artifact.relativePath);

        if (!std::filesystem::exists(path.string()))
        {
            return foundation::Result<foundation::Path>(
                foundation::Error(foundation::ErrorCode::FileNotFound, "Log artifact data not found."));
        }

        return foundation::Result<foundation::Path>(path);
    }
};

class NoteArtifactHandler final : public IArtifactHandler
{
  public:
    [[nodiscard]] std::string_view type() const override
    {
        return "note";
    }

    [[nodiscard]] foundation::Result<ArtifactRecord> ingest(const ArtifactIngestContext& context,
                                                            const ArtifactIngestRequest& request) const override
    {
        const foundation::Path dataPath = foundation::Path(context.artifactDirectory.string() + "/data");
        std::ofstream stream(dataPath.string(), std::ios::binary | std::ios::trunc);

        if (!stream)
        {
            return foundation::Result<ArtifactRecord>(
                foundation::Error(foundation::ErrorCode::IOError, "Failed to write note artifact."));
        }

        stream << request.noteBody;

        ArtifactRecord record;
        record.id = context.artifactId.empty() ? foundation::Uuid::generate().toString() : context.artifactId;
        record.type = std::string(type());
        record.name = request.name.empty() ? "note" : request.name;
        record.relativePath = "artifacts/" + record.id + "/data";
        record.importedAt = currentTimestampIso();
        record.source = request.source;
        record.source.kind = "inline";
        record.status = "ready";

        return foundation::Result<ArtifactRecord>(std::move(record));
    }

    [[nodiscard]] foundation::Result<foundation::Path> resolveDataPath(
        const foundation::Path& investigationRoot, const ArtifactRecord& artifact) const override
    {
        return foundation::Result<foundation::Path>(
            foundation::Path(investigationRoot.string() + "/" + artifact.relativePath));
    }
};

const LogArtifactHandler kLogHandler;
const NoteArtifactHandler kNoteHandler;

} // namespace

const IArtifactHandler* findArtifactHandler(const std::string_view type) noexcept
{
    if (type == "log")
    {
        return &kLogHandler;
    }

    if (type == "note")
    {
        return &kNoteHandler;
    }

    return nullptr;
}

} // namespace scope::workspace
