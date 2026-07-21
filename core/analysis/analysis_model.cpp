/**
 * @file analysis_model.cpp
 * @brief AnalysisModel implementation.
 */

#include "analysis_model.hpp"

namespace scope::analysis
{

AnalysisModel::AnalysisModel(foundation::Path sourcePath, const std::uint64_t totalLines,
                             LogLevelCounts levelCounts, const LogFormat format) noexcept
    : m_sourcePath(std::move(sourcePath))
    , m_totalLines(totalLines)
    , m_levelCounts(levelCounts)
    , m_format(format)
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

LogFormat AnalysisModel::format() const noexcept
{
    return m_format;
}

} // namespace scope::analysis
