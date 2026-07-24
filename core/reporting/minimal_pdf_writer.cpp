/**
 * @file minimal_pdf_writer.cpp
 * @brief Minimal PDF 1.4 writer for text reports and bar charts.
 */

#include "minimal_pdf_writer.hpp"

#include <algorithm>
#include <cstdio>
#include <sstream>

namespace scope::reporting
{

namespace
{

std::string escapePdfText(const std::string_view value)
{
    std::ostringstream output;

    for (const char character : value)
    {
        switch (character)
        {
        case '(':
        case ')':
        case '\\':
            output << '\\' << character;
            break;
        default:
            if (static_cast<unsigned char>(character) < 0x20U)
            {
                continue;
            }

            output << character;
            break;
        }
    }

    return output.str();
}

} // namespace

void MinimalPdfWriter::ensurePage()
{
    if (m_pageStreams.empty())
    {
        m_pageStreams.emplace_back();
        m_cursorY = 750.0F;
    }
}

void MinimalPdfWriter::advanceLine(const float lineHeight)
{
    m_cursorY -= lineHeight;

    if (m_cursorY < 72.0F)
    {
        m_pageStreams.emplace_back();
        m_cursorY = 750.0F;
    }
}

void MinimalPdfWriter::addTitle(const std::string& text)
{
    ensurePage();
    m_pageStreams.back() += "BT /F1 18 Tf 72 " + std::to_string(static_cast<int>(m_cursorY)) + " Td (" +
                           escapePdfText(text) + ") Tj ET\n";
    advanceLine(24.0F);
}

void MinimalPdfWriter::addHeading(const std::string& text)
{
    ensurePage();
    m_pageStreams.back() += "BT /F1 14 Tf 72 " + std::to_string(static_cast<int>(m_cursorY)) + " Td (" +
                           escapePdfText(text) + ") Tj ET\n";
    advanceLine(20.0F);
}

void MinimalPdfWriter::addTextLine(const std::string& text)
{
    ensurePage();
    m_pageStreams.back() += "BT /F1 11 Tf 72 " + std::to_string(static_cast<int>(m_cursorY)) + " Td (" +
                           escapePdfText(text) + ") Tj ET\n";
    advanceLine(14.0F);
}

void MinimalPdfWriter::addLevelChart(const LevelBarChart& chart)
{
    ensurePage();

    const std::uint64_t maximum = std::max<std::uint64_t>(chart.maxValue(), 1U);
    constexpr float chartLeft = 120.0F;
    constexpr float chartWidth = 360.0F;
    constexpr float barHeight = 16.0F;
    constexpr float gap = 8.0F;

    for (const ChartBar& bar : chart.bars)
    {
        if (m_cursorY < 96.0F)
        {
            m_pageStreams.emplace_back();
            m_cursorY = 750.0F;
        }

        const float barPixelWidth =
            (static_cast<float>(bar.value) * chartWidth) / static_cast<float>(maximum);

        m_pageStreams.back() += "BT /F1 11 Tf 72 " + std::to_string(static_cast<int>(m_cursorY + 4.0F)) + " Td (" +
                               escapePdfText(bar.label) + ") Tj ET\n";
        m_pageStreams.back() += "0.2 0.4 0.8 rg " + std::to_string(chartLeft) + ' ' +
                               std::to_string(m_cursorY) + ' ' + std::to_string(barPixelWidth) + ' ' +
                               std::to_string(barHeight) + " re f 0 0 0 rg\n";
        m_pageStreams.back() += "BT /F1 11 Tf " + std::to_string(chartLeft + barPixelWidth + 8.0F) + ' ' +
                               std::to_string(static_cast<int>(m_cursorY + 4.0F)) + " Td (" +
                               std::to_string(bar.value) + ") Tj ET\n";

        m_cursorY -= barHeight + gap;
    }

    advanceLine(4.0F);
}

std::vector<std::uint8_t> MinimalPdfWriter::finalize()
{
    if (m_pageStreams.empty())
    {
        m_pageStreams.emplace_back();
    }

    std::ostringstream pdf;
    std::vector<int> objectOffsets;
    const int pageCount = static_cast<int>(m_pageStreams.size());
    const int fontObjectNumber = 2 + (pageCount * 2) + 1;

    auto writeObject = [&](const std::string& body) {
        objectOffsets.push_back(static_cast<int>(pdf.tellp()));
        pdf << (objectOffsets.size()) << " 0 obj\n" << body << "\nendobj\n";
    };

    pdf << "%PDF-1.4\n";

    writeObject("<< /Type /Catalog /Pages 2 0 R >>");

    std::ostringstream kids;
    kids << "[ ";

    for (int pageIndex = 0; pageIndex < pageCount; ++pageIndex)
    {
        kids << (3 + pageIndex * 2) << " 0 R ";
    }

    kids << ']';
    writeObject("<< /Type /Pages /Kids " + kids.str() + " /Count " + std::to_string(pageCount) + " >>");

    for (int pageIndex = 0; pageIndex < pageCount; ++pageIndex)
    {
        const int pageObjectNumber = 3 + pageIndex * 2;
        const int contentObjectNumber = pageObjectNumber + 1;

        writeObject("<< /Type /Page /Parent 2 0 R /MediaBox [0 0 612 792] /Contents " +
                    std::to_string(contentObjectNumber) + " 0 R /Resources << /Font << /F1 " +
                    std::to_string(fontObjectNumber) + " 0 R >> >> >>");

        const std::string& stream = m_pageStreams[static_cast<std::size_t>(pageIndex)];
        writeObject("<< /Length " + std::to_string(stream.size()) + " >>\nstream\n" + stream + "endstream");
    }

    writeObject("<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>");

    const int xrefOffset = static_cast<int>(pdf.tellp());
    const int objectCount = static_cast<int>(objectOffsets.size());

    pdf << "xref\n0 " << (objectCount + 1) << "\n";
    pdf << "0000000000 65535 f \n";

    for (const int offset : objectOffsets)
    {
        char buffer[32];
        std::snprintf(buffer, sizeof(buffer), "%010d 00000 n \n", offset);
        pdf << buffer;
    }

    pdf << "trailer\n<< /Size " << (objectCount + 1) << " /Root 1 0 R >>\n";
    pdf << "startxref\n" << xrefOffset << "\n%%EOF\n";

    const std::string document = pdf.str();

    return std::vector<std::uint8_t>(document.begin(), document.end());
}

} // namespace scope::reporting
