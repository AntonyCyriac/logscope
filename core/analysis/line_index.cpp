/**
 * @file line_index.cpp
 * @brief LineIndex implementation.
 */

#include "line_index.hpp"

namespace scope::analysis
{

bool LineIndex::tryAddLine(IndexedLine line) noexcept
{
    if (m_lines.size() >= maxIndexedLines)
    {
        ++m_truncatedLines;

        return false;
    }

    m_lines.push_back(std::move(line));

    return true;
}

const std::vector<IndexedLine>& LineIndex::lines() const noexcept
{
    return m_lines;
}

std::uint64_t LineIndex::indexedLineCount() const noexcept
{
    return static_cast<std::uint64_t>(m_lines.size());
}

std::uint64_t LineIndex::truncatedLineCount() const noexcept
{
    return m_truncatedLines;
}

std::string truncateExcerpt(const std::string_view value, const std::size_t maxLength) noexcept
{
    if (value.size() <= maxLength)
    {
        return std::string(value);
    }

    return std::string(value.substr(0U, maxLength));
}

} // namespace scope::analysis
