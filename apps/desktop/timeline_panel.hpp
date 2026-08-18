/**
 * @file timeline_panel.hpp
 */

#pragma once

#include <QWidget>

#include <unordered_set>

#include "investigation_controller.hpp"
#include "timeline_event.hpp"
#include "viewer_navigation.hpp"

class QTableWidget;
class QLabel;
class QPushButton;
class QThread;

namespace scope::desktop
{

class RelatedEvidencePanel;
class SuggestedConnectionsPanel;
class TimelineLoadWorker;

/**
 * @brief Investigation Timeline bottom-dock tab (P2 / P2.1).
 */
class TimelinePanel : public QWidget
{
    Q_OBJECT

  public:
    explicit TimelinePanel(scope::application::InvestigationController* controller, QWidget* parent = nullptr);

    ~TimelinePanel() override;

    void setInvestigationActive(bool active);
    void setDismissedSuggestionIds(std::unordered_set<std::string>* dismissedSuggestionIds);
    void refresh();

    [[nodiscard]] bool waitForLoadComplete(int timeoutMs = 5000);
    [[nodiscard]] QString rowMessage(int row) const;
    [[nodiscard]] int rowCount() const;
    [[nodiscard]] bool selectRowByEventType(const QString& eventType);
    [[nodiscard]] bool selectRow(int row);
    [[nodiscard]] bool hasEventType(const QString& eventType) const;
    [[nodiscard]] int linkBadgeCount() const;
    [[nodiscard]] int relatedEvidenceRowCount() const;
    [[nodiscard]] bool suggestionPanelVisible() const;
    [[nodiscard]] bool createLinkBetweenRows(int sourceRow, int targetRow);
    [[nodiscard]] bool removeFirstLinkForSelection();
    [[nodiscard]] bool removeFirstEvidenceLink();
    [[nodiscard]] bool openFirstRelatedEvidence();
    [[nodiscard]] bool acceptFirstSuggestion();
    [[nodiscard]] bool dismissFirstSuggestion();

  signals:
    void navigationRequested(const scope::desktop::ViewerNavigation& navigation);
    void statusMessageRequested(const QString& message);

  private slots:
    void onLoadFinished(scope::workspace::TimelineProjectionResult result);
    void onLoadFailed(const QString& message);

  private:
    void setupUi();
    void clearTable();
    void populateTable();
    void handleRowActivated(int row);
    void startAsyncLoad();
    void refreshPanelsForSelection();
    [[nodiscard]] static QString formatEventType(const scope::workspace::TimelineEvent& event);
    [[nodiscard]] int linkCountForEvent(const std::string& eventId) const;
    [[nodiscard]] std::vector<scope::workspace::EvidenceLinkRecord> linksForActiveEvent() const;
    void handleCreateLink(const QString& targetEventId, const QString& linkType, const QString& note);
    void handleRemoveLink(const QString& linkId);
    void handleAcceptSuggestion(const QString& suggestionId);
    void handleDismissSuggestion(const QString& suggestionId);

    scope::application::InvestigationController* m_controller{nullptr};
    std::unordered_set<std::string>* m_dismissedSuggestionIds{nullptr};
    QTableWidget* m_table{nullptr};
    QLabel* m_emptyLabel{nullptr};
    QLabel* m_errorLabel{nullptr};
    QPushButton* m_loadMoreButton{nullptr};
    RelatedEvidencePanel* m_relatedPanel{nullptr};
    SuggestedConnectionsPanel* m_suggestionsPanel{nullptr};
    QThread* m_workerThread{nullptr};
    TimelineLoadWorker* m_worker{nullptr};
    std::vector<scope::workspace::TimelineEvent> m_events;
    std::vector<scope::workspace::EvidenceLinkRecord> m_allLinks;
    std::size_t m_offset{0U};
    bool m_truncated{false};
    bool m_loading{false};
    bool m_refreshPending{false};
    int m_selectedRow{-1};
};

} // namespace scope::desktop
