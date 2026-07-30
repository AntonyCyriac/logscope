/**
 * @file application_service.hpp
 * @brief Shared application orchestration for CLI and desktop (M14).
 */

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "ai_result.hpp"
#include "ai_investigation_assistant.hpp"
#include "analysis_config.hpp"
#include "analysis_model.hpp"
#include "analysis_stats.hpp"
#include "analytics_config.hpp"
#include "analytics_result.hpp"
#include "configuration_manager.hpp"
#include "extension_info.hpp"
#include "extension_manager.hpp"
#include "foundation/path.hpp"
#include "foundation/result.hpp"
#include "investigation_session.hpp"
#include "investigation_criteria.hpp"
#include "investigation_result.hpp"
#include "line_count_filter.hpp"
#include "log_level_filter.hpp"
#include "plugin_load_stats.hpp"
#include "report.hpp"
#include "report_options.hpp"
#include "search_history.hpp"
#include "source_dataset.hpp"
#include "source_open_options.hpp"

namespace scope::application
{

/**
 * @brief Result of an AI-assisted investigation request.
 */
struct AgentInvestigateResult
{
    investigation::InvestigationResult investigation;
    std::optional<ai::AiSummary> summary;
    std::optional<std::vector<ai::AiAnomalyHint>> hints;
    std::vector<std::string> aiErrors;
};

/**
 * @brief Options when saving a workspace session.
 */
struct SessionSaveRequest
{
    foundation::Path sessionFile;
    foundation::Path configFile;
    reporting::ReportOptions reportOptions;
    investigation::LineCountFilter lineFilter = investigation::LineCountFilter::any();
    investigation::LogLevelFilter levelFilter = investigation::LogLevelFilter::any();
    std::string searchQuery;
    investigation::InvestigationCriteria contentCriteria;
};

/**
 * @brief Orchestrates LogScope pipelines for CLI and desktop presentation layers.
 */
class ApplicationService
{
  public:
    [[nodiscard]] foundation::Result<bool> loadConfiguration(const foundation::Path& configFile);

    [[nodiscard]] configuration::ConfigurationManager& configurationManager() noexcept;

    [[nodiscard]] const configuration::ConfigurationManager& configurationManager() const noexcept;

    [[nodiscard]] foundation::Path configFilePath() const noexcept;

    [[nodiscard]] foundation::Path sourcePath() const noexcept;

    [[nodiscard]] bool hasModel() const noexcept;

    [[nodiscard]] const analysis::AnalysisModel& model() const;

    [[nodiscard]] foundation::Result<source::SourceDataset> openSource(const foundation::Path& path,
                                                                       source::OpenOptions options = {});

    [[nodiscard]] foundation::Result<analysis::AnalysisModel> analyze(const analysis::AnalysisConfig& config,
                                                                      analysis::AnalysisStats* statsOut = nullptr);

    [[nodiscard]] foundation::Result<investigation::InvestigationResult>
    investigate(const investigation::InvestigationCriteria& criteria);

    [[nodiscard]] foundation::Result<analytics::AnalyticsResult> runAnalytics(
        const analytics::AnalyticsConfig& config);

    [[nodiscard]] reporting::Report generateReport(const reporting::ReportOptions& options) const;

    [[nodiscard]] foundation::Result<bool> saveSession(const SessionSaveRequest& request);

    [[nodiscard]] foundation::Result<workspace::InvestigationSession> loadSession(
        const foundation::Path& sessionFile);

    [[nodiscard]] foundation::Result<std::vector<foundation::Path>> listSessions(
        const foundation::Path& directory) const;

    [[nodiscard]] std::vector<extension::ExtensionInfo> listExtensions() const;

    [[nodiscard]] foundation::Result<extension::ExtensionInfo> describeExtension(
        const std::string& extensionId) const;

    [[nodiscard]] foundation::Result<AgentInvestigateResult> agentInvestigate(
        const investigation::InvestigationCriteria& criteria,
        const std::string& askQuery,
        bool summarize,
        bool hints,
        analysis::AnalysisStats* statsOut = nullptr);

    [[nodiscard]] foundation::Result<bool> startTail();

    void stopTail();

    [[nodiscard]] bool isTailing() const noexcept;

    /**
     * @brief Reads newly appended lines during tail mode.
     */
    [[nodiscard]] foundation::Result<std::vector<std::string>> pollTailLines();

    [[nodiscard]] const plugin::PluginLoadStats& lastPluginStats() const noexcept;

    /**
     * @brief Replaces the current analysis model (e.g. after session load).
     */
    void adoptModel(const analysis::AnalysisModel& model, const foundation::Path& sourcePath);

  private:
    [[nodiscard]] extension::ExtensionManager createExtensionManager() const;

    configuration::ConfigurationManager m_configurationManager;
    foundation::Path m_configFile;
    foundation::Path m_sourcePath;
    source::OpenOptions m_openOptions{};
    std::optional<source::SourceDataset> m_dataset;
    std::optional<analysis::AnalysisModel> m_model;
    std::unique_ptr<source::LogSource> m_tailSource;
    mutable plugin::PluginLoadStats m_pluginStats;
};

} // namespace scope::application
