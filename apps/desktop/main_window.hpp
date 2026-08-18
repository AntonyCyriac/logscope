/**
 * @file main_window.hpp
 * @brief Primary LogScope desktop window.
 */

#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QStatusBar>
#include <QTableView>
#include <QTabWidget>
#include <QTextEdit>

#include "application_service.hpp"
#include "investigation_controller.hpp"
#include "analytics_panel.hpp"
#include "ai_panel.hpp"
#include "crash_panel.hpp"
#include "timeline_panel.hpp"
#include "viewer_navigation.hpp"
#include "analysis_stats.hpp"
#include "configuration_editor_dialog.hpp"
#include "desktop_analysis_config.hpp"
#include "export_report_dialog.hpp"
#include "log_format.hpp"
#include "log_table_model.hpp"
#include "open_log_dialog.hpp"
#include "run_stats_dialog.hpp"
#include "save_session_dialog.hpp"
#include <optional>
#include <unordered_set>

#include "tail_worker.hpp"

namespace scope::desktop
{

/**
 * @brief Investigation workbench main window (M14).
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    explicit MainWindow(const scope::foundation::Path& configFile = {},
                        const scope::foundation::Path& investigationsRoot = {}, QWidget* parent = nullptr);

    /// Opens and analyzes a log file (used by desktop integration tests).
    [[nodiscard]] bool openLogFile(const QString& path,
                                   scope::analysis::LogFormat formatHint = scope::analysis::LogFormat::Auto,
                                   const std::string& profile = {});

    [[nodiscard]] int logRowCount() const;
    [[nodiscard]] QString statusMessage() const;
    [[nodiscard]] QString aiOutputText() const;

    /// Sets Ask query and runs AI investigate (used by desktop integration tests).
    [[nodiscard]] bool runAiAsk(const QString& query);

    void setPersistIndexEnabled(const bool enabled);
    void setReuseIndexEnabled(const bool enabled);

    /// Investigation bar helpers for desktop integration tests.
    void setInvestigationLevel(const QString& level);
    [[nodiscard]] bool investigateCurrentFilters();

    /// Opens clipboard text via temp file (Phase C clipboard open).
    [[nodiscard]] bool openFromClipboardText(const QString& text);

    /// Saves/loads session without QFileDialog (Phase C session dialog).
    [[nodiscard]] bool saveSessionToPath(const QString& path);
    [[nodiscard]] bool loadSessionFromPath(const QString& path);

    /// Investigation mode helpers (P2 / desktop parity tests).
    [[nodiscard]] bool createInvestigation(const QString& name);
    [[nodiscard]] bool addInvestigationLogArtifact(const QString& path);
    [[nodiscard]] bool addInvestigationPstackArtifact(const QString& path);
    [[nodiscard]] bool switchBottomTab(const QString& tabName);
    [[nodiscard]] bool selectTimelineCrashSummary();
    [[nodiscard]] bool clickCrashFaultThread();
    [[nodiscard]] QString crashSignalText() const;
    [[nodiscard]] bool investigationModeActive() const;
    [[nodiscard]] bool closeActiveInvestigation();
    [[nodiscard]] bool openInvestigationAtPath(const QString& path);
    [[nodiscard]] bool isBottomTabEnabled(const QString& tabName) const;
    [[nodiscard]] int investigationArtifactCount() const;
    [[nodiscard]] QString investigationDirectoryPath() const;
    [[nodiscard]] bool timelineHasEventType(const QString& eventType) const;
    [[nodiscard]] int timelineRowCount() const;
    [[nodiscard]] QString currentBottomTabName() const;

    /// P2.1 Story Gate helpers (headless tests).
    [[nodiscard]] bool waitForTimelineLoad(int timeoutMs = 5000);
    [[nodiscard]] bool waitForCrashLoad(int timeoutMs = 5000);
    [[nodiscard]] bool selectTimelineRow(int row);
    [[nodiscard]] bool createEvidenceLinkBetweenRows(int sourceRow, int targetRow);
    [[nodiscard]] int timelineLinkBadgeCount() const;
    [[nodiscard]] int relatedEvidenceRowCount() const;
    [[nodiscard]] bool suggestionPanelVisible() const;
    [[nodiscard]] bool acceptFirstSuggestion();
    [[nodiscard]] bool dismissFirstSuggestion();
    [[nodiscard]] bool removeFirstEvidenceLink();
    [[nodiscard]] bool openFirstRelatedEvidence();

  private:
    void createMenus();
    void createLayout();
    void promptCreateInvestigation();
    void openInvestigation();
    void closeInvestigation();
    void addInvestigationArtifact();
    void refreshArtifactList();
    void setInvestigationMode(bool enabled);
    void updateInvestigationTabAvailability();
    void applyViewerNavigation(const ViewerNavigation& navigation);
    void switchToBottomTab(const QString& tabName);
    void openInvestigationLogArtifact(const QString& artifactId, std::optional<std::size_t> highlightLine);
    void onArtifactSelectionChanged();
    void loadConfigurationFile();
    void showConfigurationEditor();
    void openFile();
    void openFromClipboard();
    void openStdin();
    void runAnalyze();
    void runInvestigate();
    void runAnalytics();
    void exportReport();
    void showRunStats();
    void saveSession();
    void loadSession();
    void refreshSessions();
    void refreshExtensions();
    void showSelectedExtension();
    void toggleTail(bool enabled);
    void applyLightTheme();
    void applyDarkTheme();
    void updateStatus(const QString& message);
    void populateTableFromModel();
    void populateTableFromInvestigation(const scope::investigation::InvestigationResult& result);
    [[nodiscard]] scope::investigation::InvestigationCriteria buildInvestigationCriteriaFromUi(
        QString* errorMessage) const;

    scope::application::ApplicationService m_service;
    scope::application::InvestigationController m_investigationController;
    LogTableModel* m_logModel{nullptr};
    QTableView* m_logView{nullptr};
    QListWidget* m_sessionList{nullptr};
    QListWidget* m_artifactList{nullptr};
    QListWidget* m_extensionList{nullptr};
    QTextEdit* m_extensionDetails{nullptr};
    QLineEdit* m_searchEdit{nullptr};
    QLineEdit* m_queryEdit{nullptr};
    QLineEdit* m_filterEdit{nullptr};
    QLineEdit* m_levelEdit{nullptr};
    QLineEdit* m_timeFromEdit{nullptr};
    QLineEdit* m_timeToEdit{nullptr};
    QCheckBox* m_regexCheck{nullptr};
    QCheckBox* m_caseCheck{nullptr};
    QCheckBox* m_tailCheck{nullptr};
    QCheckBox* m_persistIndexCheck{nullptr};
    QCheckBox* m_reuseIndexCheck{nullptr};
    QTabWidget* m_bottomTabs{nullptr};
    TimelinePanel* m_timelinePanel{nullptr};
    CrashPanel* m_crashPanel{nullptr};
    AnalyticsPanel* m_analyticsPanel{nullptr};
    AiPanel* m_aiPanel{nullptr};
    TailWorker* m_tailWorker{nullptr};
    QString m_currentPath;
    scope::analysis::LogFormat m_formatHint{scope::analysis::LogFormat::Auto};
    std::string m_profile;
    bool m_hasRunStats{false};
    bool m_investigationMode{false};
    std::unordered_set<std::string> m_dismissedSuggestionIds;
    scope::analysis::AnalysisStats m_lastAnalysisStats{};
};

} // namespace scope::desktop
