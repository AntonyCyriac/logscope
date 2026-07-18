/**
 * @file report.hpp
 * @brief Represents a user-facing report (DO-003).
 */

#pragma once

#include <string>

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
     *
     * @param text Full report content.
     */
    explicit Report(std::string text);

    /**
     * @brief Returns the formatted report text.
     */
    [[nodiscard]] const std::string& text() const noexcept;

  private:
    std::string m_text;
};

} // namespace scope::reporting
