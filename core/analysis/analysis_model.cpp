/**
 * @file analysis_model.cpp
 * @brief AnalysisModel implementation.
 */

#include "analysis_model.hpp"

namespace scope::analysis
{

AnalysisModel::AnalysisModel(foundation::Path sourcePath, const std::uint64_t totalLines,
                             LogLevelCounts levelCounts) noexcept
    : m_sourcePath(std::move(sourcePath)), m_totalLines(totalLines), m_levelCounts(levelCounts)
{
}

const foundation::Path& AnalysisModel::sourcePath() const noexcept
{
    return m_sourcePath;
}

std::uint64_t AnalysisModel::totalLines() const noexcept
{
    return m_totalLines;
}

const LogLevelCounts& AnalysisModel::levelCounts() const noexcept
{
    return m_levelCounts;
}

} // namespace scope::analysis
