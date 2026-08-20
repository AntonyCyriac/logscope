/**
 * @file attributed_ingest_source.hpp
 * @brief Log source with per-line file attribution.
 */

#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "discovery_census.hpp"
#include "log_source.hpp"
#include "rotation_grouper.hpp"

namespace scope::source
{

/**
 * @brief Reads an ingest plan sequentially while tracking file and stream line numbers.
 */
class AttributedIngestSource final : public LogSource
{
  public:
    [[nodiscard]] static foundation::Result<std::unique_ptr<LogSource>> create(
        foundation::Path displayPath, std::vector<IngestStream> streams);

    [[nodiscard]] const foundation::Path& path() const noexcept override;

    [[nodiscard]] foundation::Result<bool> readLine(std::string& line) override;

    [[nodiscard]] std::optional<LineAttribution> lastLineAttribution() const noexcept;

  private:
    AttributedIngestSource(foundation::Path displayPath, std::vector<IngestStream> streams);

    [[nodiscard]] foundation::Result<bool> advanceFile();

    foundation::Path m_displayPath;
    std::vector<IngestStream> m_streams;
    std::size_t m_streamIndex{0U};
    std::size_t m_fileIndex{0U};
    std::unique_ptr<LogSource> m_currentFile;
    std::uint64_t m_fileLineNumber{0U};
    std::uint64_t m_streamLineNumber{0U};
    std::optional<LineAttribution> m_lastAttribution;
};

[[nodiscard]] bool isAttributedLogSource(const LogSource& source) noexcept;

[[nodiscard]] const AttributedIngestSource* asAttributedLogSource(const LogSource& source) noexcept;

} // namespace scope::source
