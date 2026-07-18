/**
 * @file analysis_model.cpp
 * @brief AnalysisModel implementation.
 */

#include "analysis_model.hpp"

namespace scope::analysis
{

AnalysisModel::AnalysisModel(foundation::Path sourcePath, std::uint64_t totalLines) noexcept
    : m_sourcePath(std::move(sourcePath)), m_totalLines(totalLines)
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

} // namespace scope::analysis
