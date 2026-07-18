/**
 * @file source_dataset.hpp
 * @brief Represents a prepared log source dataset (DO-001).
 */

#pragma once

#include <memory>

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

    SourceDataset(const SourceDataset&) = delete;
    SourceDataset& operator=(const SourceDataset&) = delete;

    SourceDataset(SourceDataset&&) noexcept = default;
    SourceDataset& operator=(SourceDataset&&) noexcept = default;

    /**
     * @brief Determines whether the dataset contains a source.
     */
    [[nodiscard]] bool isValid() const noexcept;

    /**
     * @brief Returns the path of the underlying source.
     */
    [[nodiscard]] const foundation::Path& path() const;

    /**
     * @brief Returns the underlying log source.
     */
    [[nodiscard]] LogSource& source();

    /**
     * @brief Returns the underlying log source.
     */
    [[nodiscard]] const LogSource& source() const;

  private:
    std::unique_ptr<LogSource> m_source;
};

} // namespace scope::source
