/**
 * @file report_fragment.hpp
 * @brief Intermediate structured content for a single report section.
 */

#pragma once

#include <string>
#include <vector>

namespace scope::reporting
{

/**
 * @brief One CSV row emitted by a report section.
 */
struct CsvRow
{
    std::string section;
    std::string key;
    std::string value;
};

/**
 * @brief Format-specific bodies for one logical report section.
 */
struct ReportFragment
{
    std::string textBody;
    std::string markdownBody;
    std::string jsonKey;
    std::string jsonBody;
    std::vector<CsvRow> csvRows;
    std::string htmlBody;

    /**
     * @brief Returns true when no format body was produced.
     */
    [[nodiscard]] bool empty() const noexcept;
};

} // namespace scope::reporting
