/**
 * @file log_level_counts.hpp
 * @brief Aggregate log level statistics from analysis.
 */

#pragma once

#include <cstdint>

namespace scope::analysis
{

/**
 * @brief Per-level line counts produced during analysis.
 */
class LogLevelCounts
{
  public:
    [[nodiscard]] std::uint64_t infoLines() const noexcept;

    [[nodiscard]] std::uint64_t warnLines() const noexcept;

    [[nodiscard]] std::uint64_t errorLines() const noexcept;

    [[nodiscard]] std::uint64_t otherLines() const noexcept;

    [[nodiscard]] std::uint64_t blankLines() const noexcept;

    void recordInfo() noexcept;

    void recordWarn() noexcept;

    void recordError() noexcept;

    void recordOther() noexcept;

    void recordBlank() noexcept;

    /**
     * @brief Creates level counts from explicit values.
     */
    [[nodiscard]] static LogLevelCounts fromCounts(std::uint64_t infoLines,
                                                   std::uint64_t warnLines,
                                                   std::uint64_t errorLines,
                                                   std::uint64_t otherLines,
                                                   std::uint64_t blankLines) noexcept;

  private:
    std::uint64_t m_infoLines{0U};
    std::uint64_t m_warnLines{0U};
    std::uint64_t m_errorLines{0U};
    std::uint64_t m_otherLines{0U};
    std::uint64_t m_blankLines{0U};
};

} // namespace scope::analysis
