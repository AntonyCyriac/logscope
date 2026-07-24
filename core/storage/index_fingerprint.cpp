/**
 * @file index_fingerprint.cpp
 */

#include "index_fingerprint.hpp"

#include <chrono>
#include <filesystem>
#include <sstream>

#include "foundation/error.hpp"
#include "foundation/filesystem.hpp"
#include "foundation/hash.hpp"

namespace scope::storage
{

namespace
{

[[nodiscard]] foundation::Result<std::int64_t> lastWriteTimeUnix(const foundation::Path& path)
{
    std::error_code error;
    const auto writeTime =
        std::filesystem::last_write_time(std::filesystem::path(path.string()), error);

    if (error)
    {
        return foundation::Result<std::int64_t>(
            foundation::Error(foundation::ErrorCode::IOError, error.message()));
    }

    const auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        writeTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());

    return foundation::Result<std::int64_t>(
        std::chrono::duration_cast<std::chrono::seconds>(sctp.time_since_epoch()).count());
}

} // namespace

const std::string& IndexFingerprint::value() const noexcept
{
    return m_value;
}

IndexFingerprint::IndexFingerprint(std::string value) noexcept
    : m_value(std::move(value))
{
}

foundation::Result<IndexFingerprint> IndexFingerprint::compute(const foundation::Path& sourcePath)
{
    const auto sizeResult = foundation::FileSystem::fileSize(sourcePath);

    if (!sizeResult)
    {
        return foundation::Result<IndexFingerprint>(sizeResult.error());
    }

    const auto mtimeResult = lastWriteTimeUnix(sourcePath);

    if (!mtimeResult)
    {
        return foundation::Result<IndexFingerprint>(mtimeResult.error());
    }

    std::ostringstream material;
    material << sourcePath.string() << '|' << *sizeResult << '|' << *mtimeResult;

    const std::uint64_t digest = foundation::hashString(material.str());

    std::ostringstream formatted;
    formatted << std::hex << digest;

    return foundation::Result<IndexFingerprint>(IndexFingerprint(formatted.str()));
}

IndexFingerprint IndexFingerprint::fromStored(std::string value)
{
    return IndexFingerprint(std::move(value));
}

foundation::Result<bool> IndexFingerprint::matchesSource(const IndexFingerprint& fingerprint,
                                                         const foundation::Path& sourcePath)
{
    const auto computed = compute(sourcePath);

    if (!computed)
    {
        return foundation::Result<bool>(computed.error());
    }

    return foundation::Result<bool>(computed->value() == fingerprint.value());
}

} // namespace scope::storage
