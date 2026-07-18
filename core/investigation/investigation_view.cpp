/**
 * @file investigation_view.cpp
 * @brief InvestigationView implementation.
 */

#include "investigation_view.hpp"

#include <sstream>

namespace scope::investigation
{

InvestigationView::InvestigationView(foundation::Path sourcePath, const std::uint64_t totalLines) noexcept
    : m_sourcePath(std::move(sourcePath)), m_totalLines(totalLines)
{
}

const foundation::Path& InvestigationView::sourcePath() const noexcept
{
    return m_sourcePath;
}

std::uint64_t InvestigationView::totalLines() const noexcept
{
    return m_totalLines;
}

bool InvestigationView::isEmpty() const noexcept
{
    return m_totalLines == 0U;
}

std::string InvestigationView::summary() const
{
    std::ostringstream output;

    output << "source=" << m_sourcePath.string() << ", lines=" << m_totalLines;

    return output.str();
}

} // namespace scope::investigation
