/**
 * @file minimal_pdf_writer.hpp
 * @brief Minimal PDF 1.4 writer for text reports and bar charts.
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "chart_model.hpp"

namespace scope::reporting
{

/**
 * @brief Generates a simple multi-page PDF document.
 */
class MinimalPdfWriter
{
  public:
    void addTitle(const std::string& text);
    void addHeading(const std::string& text);
    void addTextLine(const std::string& text);
    void addLevelChart(const LevelBarChart& chart);

    [[nodiscard]] std::vector<std::uint8_t> finalize();

  private:
    void ensurePage();
    void advanceLine(float lineHeight = 14.0F);

    std::vector<std::string> m_pageStreams;
    float m_cursorY{750.0F};
};

} // namespace scope::reporting
