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
#include "analytics_panel.hpp"
#include "ai_panel.hpp"
#include "analysis_stats.hpp"
#include "configuration_editor_dialog.hpp"
#include "desktop_analysis_config.hpp"
#include "export_report_dialog.hpp"
#include "log_format.hpp"
#include "log_table_model.hpp"
#include "open_log_dialog.hpp"
#include "run_stats_dialog.hpp"
#include "save_session_dialog.hpp"
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
    explicit MainWindow(const scope::foundation::Path& configFile = {}, QWidget* parent = nullptr);

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

  private:
    void createMenus();
    void createLayout();
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
    LogTableModel* m_logModel{nullptr};
    QTableView* m_logView{nullptr};
    QListWidget* m_sessionList{nullptr};
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
    AnalyticsPanel* m_analyticsPanel{nullptr};
    AiPanel* m_aiPanel{nullptr};
    TailWorker* m_tailWorker{nullptr};
    QString m_currentPath;
    scope::analysis::LogFormat m_formatHint{scope::analysis::LogFormat::Auto};
    std::string m_profile;
    bool m_hasRunStats{false};
    scope::analysis::AnalysisStats m_lastAnalysisStats{};
};

} // namespace scope::desktop
