/**
 * @file investigation_session.hpp
 * @brief Persisted investigation and report state (FR-002.5).
 */

#pragma once

#include <string>

#include "analysis_model.hpp"
#include "foundation/path.hpp"
#include "foundation/uuid.hpp"
#include "investigation_criteria.hpp"
#include "line_count_filter.hpp"
#include "log_level_filter.hpp"
#include "report_options.hpp"

namespace scope::workspace
{

/**
 * @brief Saved investigation context for progressive analysis workflows.
 */
class InvestigationSession
{
  public:
    InvestigationSession() = delete;

    InvestigationSession(foundation::Uuid sessionId,
                         foundation::Path sourcePath,
                         analysis::AnalysisModel model,
                         investigation::LineCountFilter lineFilter,
                         investigation::LogLevelFilter levelFilter,
                         std::string searchQuery,
                         investigation::InvestigationCriteria contentCriteria,
                         reporting::ReportOptions reportOptions,
                         foundation::Path configFile);

    [[nodiscard]] const foundation::Uuid& sessionId() const noexcept;

    [[nodiscard]] const foundation::Path& sourcePath() const noexcept;

    [[nodiscard]] const analysis::AnalysisModel& analysisModel() const noexcept;

    [[nodiscard]] const investigation::LineCountFilter& lineFilter() const noexcept;

    [[nodiscard]] const investigation::LogLevelFilter& levelFilter() const noexcept;

    [[nodiscard]] const std::string& searchQuery() const noexcept;

    [[nodiscard]] const investigation::InvestigationCriteria& contentCriteria() const noexcept;

    [[nodiscard]] const reporting::ReportOptions& reportOptions() const noexcept;

    [[nodiscard]] const foundation::Path& configFile() const noexcept;

    /**
     * @brief Creates a session from a completed analysis and user preferences.
     */
    [[nodiscard]] static InvestigationSession fromAnalysis(const analysis::AnalysisModel& model,
                                                           investigation::LineCountFilter lineFilter,
                                                           investigation::LogLevelFilter levelFilter,
                                                           std::string searchQuery,
                                                           investigation::InvestigationCriteria contentCriteria,
                                                           reporting::ReportOptions reportOptions,
                                                           foundation::Path configFile);

  private:
    foundation::Uuid m_sessionId;
    foundation::Path m_sourcePath;
    analysis::AnalysisModel m_analysisModel;
    investigation::LineCountFilter m_lineFilter;
    investigation::LogLevelFilter m_levelFilter;
    std::string m_searchQuery;
    investigation::InvestigationCriteria m_contentCriteria;
    reporting::ReportOptions m_reportOptions;
    foundation::Path m_configFile;
};

} // namespace scope::workspace
