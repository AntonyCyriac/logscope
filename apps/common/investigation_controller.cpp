/**
 * @file investigation_controller.cpp
 */

#include "investigation_controller.hpp"

#include "artifact_handler.hpp"
#include "foundation/uuid.hpp"
#include "investigation_manifest_io.hpp"

#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>

namespace scope::application
{

namespace
{

using scope::foundation::Path;
using scope::workspace::ArtifactIngestRequest;
using scope::workspace::ArtifactSource;
using scope::workspace::Investigation;
using scope::workspace::InvestigationCreateRequest;

bool isValidInvestigationId(const std::string& investigationId)
{
    static const std::regex pattern("^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$");

    return std::regex_match(investigationId, pattern)
           && static_cast<bool>(scope::foundation::Uuid::parse(investigationId));
}

} // namespace

InvestigationController::InvestigationController(const Path investigationsRootDirectory)
    : m_rootDirectory(investigationsRootDirectory)
{
    std::error_code errorCode;
    std::filesystem::create_directories(m_rootDirectory.string(), errorCode);
}

const Path& InvestigationController::investigationsRootDirectory() const noexcept
{
    return m_rootDirectory;
}

bool InvestigationController::isOpen() const noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_investigation.has_value();
}

Path InvestigationController::investigationDirectory() const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_investigation.has_value())
    {
        return {};
    }

    return m_investigation->rootDirectory();
}

const scope::workspace::InvestigationManifest& InvestigationController::manifest() const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_investigation.has_value())
    {
        static const scope::workspace::InvestigationManifest emptyManifest{};

        return emptyManifest;
    }

    return m_investigation->manifest();
}

Path InvestigationController::investigationDirectoryForId(const std::string& investigationId) const
{
    return Path(m_rootDirectory.string() + "/" + investigationId);
}

foundation::Result<scope::workspace::InvestigationManifest> InvestigationController::create(const std::string& name,
                                                                                             const std::string& description)
{
    if (name.empty() || name.size() > 256U)
    {
        return foundation::Result<scope::workspace::InvestigationManifest>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Investigation name is required (max 256 characters)."));
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    const Path stagingDir =
        Path(m_rootDirectory.string() + "/.staging-" + foundation::Uuid::generate().toString());

    InvestigationCreateRequest createRequest;
    createRequest.name = name;
    createRequest.description = description;

    const auto createResult = Investigation::create(stagingDir, createRequest);

    if (!createResult)
    {
        return foundation::Result<scope::workspace::InvestigationManifest>(createResult.error());
    }

    const std::string investigationId = createResult->manifest().id;
    const Path investigationDir = investigationDirectoryForId(investigationId);

    std::error_code errorCode;
    std::filesystem::rename(stagingDir.string(), investigationDir.string(), errorCode);

    if (errorCode)
    {
        std::filesystem::remove_all(stagingDir.string(), errorCode);

        return foundation::Result<scope::workspace::InvestigationManifest>(
            foundation::Error(foundation::ErrorCode::IOError, "Failed to finalize investigation directory."));
    }

    const auto openResult = Investigation::open(investigationDir);

    if (!openResult)
    {
        return foundation::Result<scope::workspace::InvestigationManifest>(openResult.error());
    }

    m_investigation = std::move(*openResult);

    return foundation::Result<scope::workspace::InvestigationManifest>(m_investigation->manifest());
}

foundation::Result<scope::workspace::InvestigationManifest> InvestigationController::open(const Path& investigationDir)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    const auto openResult = Investigation::open(investigationDir);

    if (!openResult)
    {
        return foundation::Result<scope::workspace::InvestigationManifest>(openResult.error());
    }

    m_investigation = std::move(*openResult);

    return foundation::Result<scope::workspace::InvestigationManifest>(m_investigation->manifest());
}

void InvestigationController::close()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_investigation.reset();
}

foundation::Result<scope::workspace::ArtifactRecord> InvestigationController::addLogArtifact(
    const Path& sourcePath, const std::string& displayName)
{
    return addArtifactFile(sourcePath, "log", displayName);
}

foundation::Result<scope::workspace::ArtifactRecord> InvestigationController::addArtifactFile(
    const Path& sourcePath, const std::string& type, const std::string& displayName)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_investigation.has_value())
    {
        return foundation::Result<scope::workspace::ArtifactRecord>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "No investigation is open."));
    }

    ArtifactIngestRequest request;
    request.type = type.empty() ? inferArtifactType({}, sourcePath) : type;
    request.name = displayName.empty() ? sourcePath.filename().string() : displayName;
    request.sourceFile = sourcePath;
    request.source = ArtifactSource{"upload", request.name};

    const auto artifactResult = m_investigation->addArtifact(request);

    if (!artifactResult)
    {
        return foundation::Result<scope::workspace::ArtifactRecord>(artifactResult.error());
    }

    const auto persistResult = m_investigation->persist();

    if (!persistResult)
    {
        return foundation::Result<scope::workspace::ArtifactRecord>(persistResult.error());
    }

    return foundation::Result<scope::workspace::ArtifactRecord>(*artifactResult);
}

foundation::Result<bool> InvestigationController::removeArtifact(const std::string& artifactId)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_investigation.has_value())
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "No investigation is open."));
    }

    const auto removeResult = m_investigation->removeArtifact(artifactId);

    if (!removeResult)
    {
        return removeResult;
    }

    return m_investigation->persist();
}

foundation::Result<Investigation> InvestigationController::openInvestigation() const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_investigation.has_value())
    {
        return foundation::Result<Investigation>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "No investigation is open."));
    }

    return Investigation::open(m_investigation->rootDirectory());
}

foundation::Result<scope::workspace::TimelineProjectionResult> InvestigationController::projectTimeline(
    scope::workspace::TimelineProjectionOptions options) const
{
    const auto investigationResult = openInvestigation();

    if (!investigationResult)
    {
        return foundation::Result<scope::workspace::TimelineProjectionResult>(investigationResult.error());
    }

    return investigationResult->projectTimeline(options);
}

foundation::Result<scope::workspace::CrashReport> InvestigationController::analyzeCrash(
    const std::string& artifactId) const
{
    const auto investigationResult = openInvestigation();

    if (!investigationResult)
    {
        return foundation::Result<scope::workspace::CrashReport>(investigationResult.error());
    }

    return investigationResult->analyzeCrash(artifactId);
}

foundation::Result<std::vector<scope::workspace::EvidenceLinkRecord>> InvestigationController::listEvidenceLinks()
    const
{
    const auto investigationResult = openInvestigation();

    if (!investigationResult)
    {
        return foundation::Result<std::vector<scope::workspace::EvidenceLinkRecord>>(investigationResult.error());
    }

    return investigationResult->listEvidenceLinks();
}

foundation::Result<Path> InvestigationController::resolveLogArtifactPath(const std::string& artifactId) const
{
    const auto investigationResult = openInvestigation();

    if (!investigationResult)
    {
        return foundation::Result<Path>(investigationResult.error());
    }

    if (artifactId.empty())
    {
        return investigationResult->entryArtifactDataPath();
    }

    return investigationResult->logArtifactDataPath(artifactId);
}

foundation::Result<Path> InvestigationController::resolveArtifactDataPath(const std::string& artifactId) const
{
    const auto investigationResult = openInvestigation();

    if (!investigationResult)
    {
        return foundation::Result<Path>(investigationResult.error());
    }

    const auto artifactResult = investigationResult->artifactById(artifactId);

    if (!artifactResult)
    {
        return foundation::Result<Path>(artifactResult.error());
    }

    const scope::workspace::IArtifactHandler* handler = scope::workspace::findArtifactHandler(artifactResult->type);

    if (handler == nullptr)
    {
        return foundation::Result<Path>(foundation::Error(foundation::ErrorCode::InvalidArgument,
                                                          "Unsupported artifact type."));
    }

    return handler->resolveDataPath(investigationResult->rootDirectory(), *artifactResult);
}

foundation::Result<std::string> InvestigationController::readArtifactText(const std::string& artifactId) const
{
    const auto pathResult = resolveArtifactDataPath(artifactId);

    if (!pathResult)
    {
        return foundation::Result<std::string>(pathResult.error());
    }

    std::ifstream stream(pathResult->string(), std::ios::binary);

    if (!stream)
    {
        return foundation::Result<std::string>(
            foundation::Error(foundation::ErrorCode::IOError, "Could not read artifact data."));
    }

    std::ostringstream buffer;
    buffer << stream.rdbuf();

    return foundation::Result<std::string>(buffer.str());
}

std::string InvestigationController::inferArtifactType(const std::string& explicitType, const Path& sourceFile)
{
    if (!explicitType.empty())
    {
        return explicitType;
    }

    const std::string filename = sourceFile.filename().string();
    const std::size_t dot = filename.rfind('.');

    if (dot != std::string::npos)
    {
        const std::string extension = filename.substr(dot);

        if (extension == ".core")
        {
            return "core";
        }

        if (extension == ".pstack")
        {
            return "pstack";
        }
    }

    return "log";
}

} // namespace scope::application
