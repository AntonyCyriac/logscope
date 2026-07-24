/**
 * @file report.hpp
 * @brief Represents a user-facing report (DO-003).
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace scope::reporting
{

/**
 * @brief Presentation of analysis results for users or external systems.
 */
class Report
{
  public:
    /**
     * @brief Constructs a report from formatted text.
     */
    explicit Report(std::string text);

    /**
     * @brief Constructs a report from binary content.
     */
    static Report fromBinary(std::vector<std::uint8_t> bytes, std::string mimeType);

    /**
     * @brief Returns true when the report payload is binary.
     */
    [[nodiscard]] bool isBinary() const noexcept;

    /**
     * @brief Returns the formatted report text for text-based formats.
     */
    [[nodiscard]] const std::string& text() const noexcept;

    /**
     * @brief Returns binary report bytes for PDF and similar formats.
     */
    [[nodiscard]] const std::vector<std::uint8_t>& bytes() const noexcept;

    /**
     * @brief Returns the MIME type for binary reports.
     */
    [[nodiscard]] const std::string& mimeType() const noexcept;

  private:
    bool m_isBinary{false};
    std::string m_text;
    std::vector<std::uint8_t> m_bytes;
    std::string m_mimeType;
};

} // namespace scope::reporting
