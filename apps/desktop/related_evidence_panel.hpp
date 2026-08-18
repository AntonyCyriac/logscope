/**
 * @file related_evidence_panel.hpp
 * @brief Story 5 Related Evidence panel (P2.1).
 */

#pragma once

#include <QWidget>

#include "viewer_navigation.hpp"

#include <vector>

#include "evidence_link.hpp"
#include "timeline_event.hpp"

class QComboBox;
class QLabel;
class QListWidget;
class QPushButton;

namespace scope::desktop
{

struct ViewerNavigation;

/**
 * @brief Lists and manages evidence links for the selected timeline event.
 */
class RelatedEvidencePanel : public QWidget
{
    Q_OBJECT

  public:
    explicit RelatedEvidencePanel(QWidget* parent = nullptr);

    void setActiveEvent(const scope::workspace::TimelineEvent* event,
                        const std::vector<scope::workspace::EvidenceLinkRecord>& links,
                        const std::vector<scope::workspace::TimelineEvent>& timelineEvents);

    void clear();

    [[nodiscard]] int rowCount() const;
    [[nodiscard]] bool openFirstRow();

  signals:
    void createLinkRequested(const QString& targetEventId, const QString& linkType, const QString& note);
    void removeLinkRequested(const QString& linkId);
    void openLinkRequested(const ViewerNavigation& navigation);

  private:
    void setupUi();
    void renderLinks();
    void populateTargetChoices();
    [[nodiscard]] const scope::workspace::TimelineEvent* findEvent(const std::string& eventId) const;

    const scope::workspace::TimelineEvent* m_activeEvent{nullptr};
    std::vector<scope::workspace::EvidenceLinkRecord> m_links;
    std::vector<scope::workspace::TimelineEvent> m_timelineEvents;

    QLabel* m_titleLabel{nullptr};
    QLabel* m_emptyLabel{nullptr};
    QListWidget* m_list{nullptr};
    QPushButton* m_addButton{nullptr};
    QWidget* m_createForm{nullptr};
    QComboBox* m_targetCombo{nullptr};
    QComboBox* m_typeCombo{nullptr};
    QPushButton* m_createButton{nullptr};
    QPushButton* m_cancelButton{nullptr};
    bool m_createMode{false};
};

} // namespace scope::desktop
