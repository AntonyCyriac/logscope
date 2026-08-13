/**
 * @file artifact_handler.cpp
 */

#include "artifact_handler.hpp"

#include "foundation/clock.hpp"
#include "foundation/uuid.hpp"

#include <chrono>
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

std::optional<std::string> captureSourceFileModifiedAt(const foundation::Path& sourceFile)
{
    std::error_code errorCode;
    const auto fileTime = std::filesystem::last_write_time(sourceFile.string(), errorCode);

    if (errorCode)
    {
        return std::nullopt;
    }

    const auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        fileTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
    const auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(sctp.time_since_epoch()).count();
    return foundation::Timestamp::fromUnixSeconds(seconds).toString();
}

void applySourceModifiedAt(ArtifactRecord& record, const foundation::Path& sourceFile)
{
    if (const std::optional<std::string> modifiedAt = captureSourceFileModifiedAt(sourceFile))
    {
        record.sourceModifiedAt = *modifiedAt;
    }
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
        applySourceModifiedAt(record, request.sourceFile);
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

class PstackArtifactHandler final : public IArtifactHandler
{
  public:
    [[nodiscard]] std::string_view type() const override
    {
        return "pstack";
    }

    [[nodiscard]] foundation::Result<ArtifactRecord> ingest(const ArtifactIngestContext& context,
                                                            const ArtifactIngestRequest& request) const override
    {
        if (request.sourceFile.string().empty())
        {
            return foundation::Result<ArtifactRecord>(foundation::Error(
                foundation::ErrorCode::InvalidArgument, "Pstack artifact requires a source file."));
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
        applySourceModifiedAt(record, request.sourceFile);
        record.source = request.source;
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

class CoreArtifactHandler final : public IArtifactHandler
{
  public:
    [[nodiscard]] std::string_view type() const override
    {
        return "core";
    }

    [[nodiscard]] foundation::Result<ArtifactRecord> ingest(const ArtifactIngestContext& context,
                                                            const ArtifactIngestRequest& request) const override
    {
        if (request.sourceFile.string().empty())
        {
            return foundation::Result<ArtifactRecord>(foundation::Error(
                foundation::ErrorCode::InvalidArgument, "Core artifact requires a source file."));
        }

        const foundation::Path dataPath = foundation::Path(context.artifactDirectory.string() + "/data");
        const auto copyResult = copyFileTo(request.sourceFile, dataPath);

        if (!copyResult)
        {
            return foundation::Result<ArtifactRecord>(copyResult.error());
        }

        std::error_code errorCode;
        const auto sizeBytes = std::filesystem::file_size(dataPath.string(), errorCode);

        ArtifactRecord record;
        record.id = context.artifactId.empty() ? foundation::Uuid::generate().toString() : context.artifactId;
        record.type = std::string(type());
        record.name = request.name.empty() ? request.sourceFile.filename().string() : request.name;
        record.relativePath = "artifacts/" + record.id + "/data";
        record.importedAt = currentTimestampIso();
        applySourceModifiedAt(record, request.sourceFile);
        record.source = request.source;
        record.status = "ready";

        if (!errorCode)
        {
            record.metadata["sizeBytes"] = std::to_string(sizeBytes);
        }

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
const PstackArtifactHandler kPstackHandler;
const CoreArtifactHandler kCoreHandler;

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

    if (type == "pstack")
    {
        return &kPstackHandler;
    }

    if (type == "core")
    {
        return &kCoreHandler;
    }

    return nullptr;
}

bool artifactTypeSupportsSessionOpen(const std::string_view type) noexcept
{
    return type == "log";
}

} // namespace scope::workspace
