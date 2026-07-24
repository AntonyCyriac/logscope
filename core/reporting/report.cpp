/**
 * @file report.cpp
 * @brief Report implementation.
 */

#include "report.hpp"

namespace scope::reporting
{

Report::Report(std::string text) : m_text(std::move(text))
{
}

Report Report::fromBinary(std::vector<std::uint8_t> bytes, std::string mimeType)
{
    Report report("");
    report.m_isBinary = true;
    report.m_bytes = std::move(bytes);
    report.m_mimeType = std::move(mimeType);

    return report;
}

bool Report::isBinary() const noexcept
{
    return m_isBinary;
}

const std::string& Report::text() const noexcept
{
    return m_text;
}

const std::vector<std::uint8_t>& Report::bytes() const noexcept
{
    return m_bytes;
}

const std::string& Report::mimeType() const noexcept
{
    return m_mimeType;
}

} // namespace scope::reporting
