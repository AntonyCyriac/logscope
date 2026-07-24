/**
 * @file report_section.hpp
 * @brief Selectable report sections for FR-003.2.
 */

#pragma once

#include <optional>
#include <string_view>

namespace scope::reporting
{

/**
 * @brief Logical sections that may appear in a report.
 */
enum class ReportSection
{
    ExecutiveSummary,
    Summary,
    LevelBreakdown,
    ErrorSummary,
    Charts,
    SourceMetadata,
    FormatsFooter
};

/**
 * @brief Enabled report sections.
 */
class ReportSections
{
  public:
    /**
     * @brief Enables all supported sections.
     */
    [[nodiscard]] static ReportSections all() noexcept;

    /**
     * @brief Parses a comma-separated section list.
     *
     * Supported names: executive, summary, levels, errors, charts, metadata, all.
     */
    [[nodiscard]] static std::optional<ReportSections> parse(std::string_view value);

    ReportSections() = default;

    /**
     * @brief Determines whether a section is enabled.
     */
    [[nodiscard]] bool includes(ReportSection section) const noexcept;

    /**
     * @brief Enables a section.
     */
    void enable(ReportSection section) noexcept;

    /**
     * @brief Disables a section.
     */
    void disable(ReportSection section) noexcept;

  private:
    explicit ReportSections(unsigned mask) noexcept;

    unsigned m_mask = 0U;
};

} // namespace scope::reporting
