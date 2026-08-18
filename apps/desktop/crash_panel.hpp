/**
 * @file crash_panel.hpp
 */

#pragma once

#include <QWidget>

#include <QListWidget>

#include "crash_report.hpp"
#include "investigation_controller.hpp"
#include "viewer_navigation.hpp"

class QComboBox;
class QLabel;
class QTextBrowser;
class QThread;

namespace scope::desktop
{

class CrashLoadWorker;

/**
 * @brief Investigation Crash bottom-dock tab (P2).
 */
class CrashPanel : public QWidget
{
    Q_OBJECT

  public:
    explicit CrashPanel(scope::application::InvestigationController* controller, QWidget* parent = nullptr);

    ~CrashPanel() override;

    void setInvestigationActive(bool active);
    void refresh();
    void setActiveArtifactId(const QString& artifactId);

    [[nodiscard]] QString signalText() const;
    [[nodiscard]] bool clickFaultThread();
    [[nodiscard]] bool waitForLoadComplete(int timeoutMs = 5000);

  signals:
    void navigationRequested(const scope::desktop::ViewerNavigation& navigation);

  private slots:
    void onLoadFinished(scope::workspace::CrashReport report);
    void onLoadFailed(const QString& message);

  private:
    void setupUi();
    void populateArtifactChoices();
    void renderReport(const scope::workspace::CrashReport& report);
    void renderPstackViewer(const QString& body, const QString& highlightThreadId);
    void handleThreadActivated(QListWidgetItem* item);
    [[nodiscard]] QString currentArtifactId() const;

    scope::application::InvestigationController* m_controller{nullptr};
    QComboBox* m_artifactCombo{nullptr};
    QLabel* m_signalLabel{nullptr};
    QLabel* m_summaryLabel{nullptr};
    QLabel* m_emptyLabel{nullptr};
    QLabel* m_errorLabel{nullptr};
    QListWidget* m_threadList{nullptr};
    QTextBrowser* m_pstackViewer{nullptr};
    scope::workspace::CrashReport m_report{};
    QString m_pstackBody;
    QString m_activeArtifactId;
    QString m_highlightThreadId;
    bool m_loading{false};
    QThread* m_workerThread{nullptr};
    CrashLoadWorker* m_worker{nullptr};
    bool m_refreshPending{false};

    void startAsyncLoad();
};

} // namespace scope::desktop
