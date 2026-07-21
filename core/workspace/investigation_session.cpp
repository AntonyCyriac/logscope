/**
 * @file investigation_session.cpp
 * @brief InvestigationSession implementation.
 */

#include "investigation_session.hpp"

namespace scope::workspace
{

InvestigationSession::InvestigationSession(
    foundation::Uuid sessionId, foundation::Path sourcePath, analysis::AnalysisModel model,
    investigation::LineCountFilter lineFilter, investigation::LogLevelFilter levelFilter,
    std::string searchQuery, investigation::InvestigationCriteria contentCriteria,
    search::SearchHistory searchHistory, reporting::ReportOptions reportOptions, foundation::Path configFile)
    : m_sessionId(std::move(sessionId))
    , m_sourcePath(std::move(sourcePath))
    , m_analysisModel(std::move(model))
    , m_lineFilter(std::move(lineFilter))
    , m_levelFilter(std::move(levelFilter))
    , m_searchQuery(std::move(searchQuery))
    , m_contentCriteria(std::move(contentCriteria))
    , m_searchHistory(std::move(searchHistory))
    , m_reportOptions(std::move(reportOptions))
    , m_configFile(std::move(configFile))
{
}

const foundation::Uuid& InvestigationSession::sessionId() const noexcept
{
    return m_sessionId;
}

const foundation::Path& InvestigationSession::sourcePath() const noexcept
{
    return m_sourcePath;
}

const analysis::AnalysisModel& InvestigationSession::analysisModel() const noexcept
{
    return m_analysisModel;
}

const investigation::LineCountFilter& InvestigationSession::lineFilter() const noexcept
{
    return m_lineFilter;
}

const investigation::LogLevelFilter& InvestigationSession::levelFilter() const noexcept
{
    return m_levelFilter;
}

const std::string& InvestigationSession::searchQuery() const noexcept
{
    return m_searchQuery;
}

const investigation::InvestigationCriteria& InvestigationSession::contentCriteria() const noexcept
{
    return m_contentCriteria;
}

const search::SearchHistory& InvestigationSession::searchHistory() const noexcept
{
    return m_searchHistory;
}

const reporting::ReportOptions& InvestigationSession::reportOptions() const noexcept
{
    return m_reportOptions;
}

const foundation::Path& InvestigationSession::configFile() const noexcept
{
    return m_configFile;
}

InvestigationSession InvestigationSession::fromAnalysis(const analysis::AnalysisModel& model,
                                                        investigation::LineCountFilter lineFilter,
                                                        investigation::LogLevelFilter levelFilter,
                                                        std::string searchQuery,
                                                        investigation::InvestigationCriteria contentCriteria,
                                                        search::SearchHistory searchHistory,
                                                        reporting::ReportOptions reportOptions,
                                                        foundation::Path configFile)
{
    const foundation::Uuid sessionId = foundation::Uuid::generate();

    return InvestigationSession(sessionId, model.sourcePath(), model, std::move(lineFilter),
                                std::move(levelFilter), std::move(searchQuery), std::move(contentCriteria),
                                std::move(searchHistory), std::move(reportOptions), std::move(configFile));
}

} // namespace scope::workspace
