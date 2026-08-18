/**
 * @file timeline_panel.hpp
 */

#pragma once

#include <QWidget>

#include "investigation_controller.hpp"
#include "timeline_event.hpp"
#include "viewer_navigation.hpp"

class QTableWidget;
class QLabel;
class QPushButton;
class QThread;

namespace scope::desktop
{

class TimelineLoadWorker;

/**
 * @brief Investigation Timeline bottom-dock tab (P2).
 */
class TimelinePanel : public QWidget
{
    Q_OBJECT

  public:
    explicit TimelinePanel(scope::application::InvestigationController* controller, QWidget* parent = nullptr);

    ~TimelinePanel() override;

    void setInvestigationActive(bool active);
    void refresh();

    [[nodiscard]] QString rowMessage(int row) const;
    [[nodiscard]] int rowCount() const;
    [[nodiscard]] bool selectRowByEventType(const QString& eventType);
    [[nodiscard]] bool hasEventType(const QString& eventType) const;

  signals:
    void navigationRequested(const scope::desktop::ViewerNavigation& navigation);

  private:
    void setupUi();
    void clearTable();
    void populateTable(const scope::workspace::TimelineProjectionResult& result);
    void handleRowActivated(int row);
    [[nodiscard]] static QString formatEventType(const scope::workspace::TimelineEvent& event);

    scope::application::InvestigationController* m_controller{nullptr};
    QTableWidget* m_table{nullptr};
    QLabel* m_emptyLabel{nullptr};
    QLabel* m_errorLabel{nullptr};
    QPushButton* m_loadMoreButton{nullptr};
    std::vector<scope::workspace::TimelineEvent> m_events;
    std::size_t m_offset{0U};
    bool m_truncated{false};
    bool m_loading{false};
};

} // namespace scope::desktop
