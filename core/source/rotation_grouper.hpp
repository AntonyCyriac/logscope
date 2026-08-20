/**
 * @file rotation_grouper.hpp
 * @brief Groups rotated log files into logical streams.
 */

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "discovery_census.hpp"
#include "foundation/path.hpp"

namespace scope::source
{

struct IngestFile
{
    std::string relativePath;
    foundation::Path absolutePath;
    std::string instanceKey;
};

struct IngestStream
{
    std::string streamId;
    std::string rotationGroupId;
    std::string instanceKey;
    std::vector<IngestFile> orderedFiles;
};

[[nodiscard]] std::vector<IngestStream> buildIngestStreams(const foundation::Path& root,
                                                          const std::vector<IngestFile>& textCandidates);

[[nodiscard]] std::optional<std::string> rotationGroupIdForPath(std::string_view relativePath) noexcept;

} // namespace scope::source
