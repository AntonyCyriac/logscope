/**
 * @file investigation_view.cpp
 * @brief InvestigationView implementation.
 */

#include "investigation_view.hpp"

#include <sstream>

namespace scope::investigation
{

InvestigationView::InvestigationView(const analysis::AnalysisModel& model) noexcept
    : m_sourcePath(model.sourcePath()), m_totalLines(model.totalLines()), m_levelCounts(model.levelCounts())
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

const analysis::LogLevelCounts& InvestigationView::levelCounts() const noexcept
{
    return m_levelCounts;
}

bool InvestigationView::isEmpty() const noexcept
{
    return m_totalLines == 0U;
}

std::string InvestigationView::summary() const
{
    std::ostringstream output;

    output << "source=" << m_sourcePath.string() << ", lines=" << m_totalLines << ", errors="
           << m_levelCounts.errorLines() << ", warnings=" << m_levelCounts.warnLines()
           << ", info=" << m_levelCounts.infoLines();

    return output.str();
}

} // namespace scope::investigation
