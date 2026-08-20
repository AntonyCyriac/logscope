/**
 * @file source_dataset.hpp
 * @brief Represents a prepared log source dataset (DO-001).
 */

#pragma once

#include <memory>
#include <optional>

#include "discovery_census.hpp"
#include "foundation/path.hpp"
#include "log_source.hpp"

namespace scope::source
{

/**
 * @brief Logical collection of log data prepared for analysis.
 */
class SourceDataset
{
  public:
    SourceDataset() = default;

    explicit SourceDataset(std::unique_ptr<LogSource> source);

    SourceDataset(std::unique_ptr<LogSource> source, DiscoveryCensus census);

    SourceDataset(const SourceDataset&) = delete;
    SourceDataset& operator=(const SourceDataset&) = delete;

    SourceDataset(SourceDataset&&) noexcept = default;
    SourceDataset& operator=(SourceDataset&&) noexcept = default;

    [[nodiscard]] bool isValid() const noexcept;

    [[nodiscard]] const foundation::Path& path() const;

    [[nodiscard]] LogSource& source();

    [[nodiscard]] const LogSource& source() const;

    [[nodiscard]] bool hasDiscoveryCensus() const noexcept;

    [[nodiscard]] const DiscoveryCensus& discoveryCensus() const noexcept;

    [[nodiscard]] AnalysisAccounting& analysisAccounting() noexcept;

    [[nodiscard]] const AnalysisAccounting& analysisAccounting() const noexcept;

  private:
    std::unique_ptr<LogSource> m_source;
    std::optional<DiscoveryCensus> m_discoveryCensus;
    AnalysisAccounting m_analysisAccounting;
};

} // namespace scope::source
