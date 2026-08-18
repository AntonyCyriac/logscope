/**
 * @file related_evidence_panel.cpp
 */

#include "related_evidence_panel.hpp"

#include "evidence_link.hpp"
#include "viewer_navigation.hpp"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

namespace scope::desktop
{

namespace
{

QString linkTypeLabel(const scope::workspace::EvidenceLinkType type)
{
    switch (type)
    {
    case scope::workspace::EvidenceLinkType::Precedes:
        return QStringLiteral("Precedes");
    case scope::workspace::EvidenceLinkType::Follows:
        return QStringLiteral("Follows");
    case scope::workspace::EvidenceLinkType::Supports:
        return QStringLiteral("Supports");
    case scope::workspace::EvidenceLinkType::Related:
    default:
        return QStringLiteral("Related");
    }
}

} // namespace

RelatedEvidencePanel::RelatedEvidencePanel(QWidget* parent) : QWidget(parent)
{
    setupUi();
}

void RelatedEvidencePanel::setupUi()
{
    auto* layout = new QVBoxLayout(this);

    m_titleLabel = new QLabel(QStringLiteral("Related Evidence"), this);
    m_titleLabel->setObjectName(QStringLiteral("related-evidence-title"));

    m_emptyLabel = new QLabel(QStringLiteral("No related evidence"), this);
    m_emptyLabel->setObjectName(QStringLiteral("related-evidence-empty"));

    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("related-evidence-panel"));

    m_addButton = new QPushButton(QStringLiteral("Add connection"), this);
    m_addButton->setObjectName(QStringLiteral("related-evidence-add"));

    auto* removeButton = new QPushButton(QStringLiteral("Remove selected"), this);
    removeButton->setObjectName(QStringLiteral("related-evidence-remove"));

    m_createForm = new QWidget(this);
    m_createForm->setObjectName(QStringLiteral("related-evidence-create-form"));
    auto* formLayout = new QVBoxLayout(m_createForm);

    m_targetCombo = new QComboBox(m_createForm);
    m_typeCombo = new QComboBox(m_createForm);
    m_typeCombo->addItem(QStringLiteral("Related"), QStringLiteral("RELATED"));
    m_typeCombo->addItem(QStringLiteral("Supports"), QStringLiteral("SUPPORTS"));
    m_typeCombo->addItem(QStringLiteral("Precedes"), QStringLiteral("PRECEDES"));
    m_typeCombo->addItem(QStringLiteral("Follows"), QStringLiteral("FOLLOWS"));

    auto* buttonRow = new QHBoxLayout();
    m_createButton = new QPushButton(QStringLiteral("Create"), m_createForm);
    m_cancelButton = new QPushButton(QStringLiteral("Cancel"), m_createForm);
    buttonRow->addWidget(m_createButton);
    buttonRow->addWidget(m_cancelButton);

    formLayout->addWidget(new QLabel(QStringLiteral("Target event"), m_createForm));
    formLayout->addWidget(m_targetCombo);
    formLayout->addWidget(new QLabel(QStringLiteral("Type"), m_createForm));
    formLayout->addWidget(m_typeCombo);
    formLayout->addLayout(buttonRow);
    m_createForm->setVisible(false);

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_emptyLabel);
    layout->addWidget(m_list, 1);
    layout->addWidget(m_addButton);
    layout->addWidget(removeButton);
    layout->addWidget(m_createForm);

    connect(removeButton, &QPushButton::clicked, this, [this]() {
        QListWidgetItem* item = m_list->currentItem();

        if (item == nullptr)
        {
            return;
        }

        emit removeLinkRequested(item->data(Qt::UserRole).toString());
    });

    connect(m_addButton, &QPushButton::clicked, this, [this]() {
        m_createMode = true;
        populateTargetChoices();
        m_createForm->setVisible(true);
        m_addButton->setVisible(false);
    });

    connect(m_cancelButton, &QPushButton::clicked, this, [this]() {
        m_createMode = false;
        m_createForm->setVisible(false);
        m_addButton->setVisible(m_activeEvent != nullptr);
    });

    connect(m_createButton, &QPushButton::clicked, this, [this]() {
        if (m_activeEvent == nullptr || m_targetCombo->currentData().toString().isEmpty())
        {
            return;
        }

        emit createLinkRequested(m_targetCombo->currentData().toString(), m_typeCombo->currentData().toString(),
                                 QString{});

        m_createMode = false;
        m_createForm->setVisible(false);
        m_addButton->setVisible(true);
    });

    connect(m_list, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (item == nullptr || m_activeEvent == nullptr)
        {
            return;
        }

        const QString linkId = item->data(Qt::UserRole).toString();
        const QString peerEventId = item->data(Qt::UserRole + 1).toString();
        const QString artifactId = item->data(Qt::UserRole + 2).toString();
        const QVariant lineValue = item->data(Qt::UserRole + 3);

        ViewerNavigation navigation;
        navigation.peerEventId = peerEventId;
        navigation.artifactId = artifactId;

        const auto* peer = findEvent(peerEventId.toStdString());

        if (peer != nullptr && peer->eventType == "crash.summary")
        {
            navigation.targetTab = QStringLiteral("crash");
            navigation.statusMessage = QStringLiteral("Opened linked crash evidence");
        }
        else if (peer != nullptr && peer->eventType == "log.line")
        {
            navigation.targetTab = QStringLiteral("results");
            navigation.artifactId = QString::fromStdString(peer->source.artifactId);

            if (peer->source.lineNumber.has_value())
            {
                navigation.lineNumber = *peer->source.lineNumber;
            }

            navigation.statusMessage = QStringLiteral("Opened linked log line");
        }
        else
        {
            navigation.targetTab = QStringLiteral("timeline");
            navigation.statusMessage = QStringLiteral("Opened linked event");
        }

        Q_UNUSED(linkId);
        emit openLinkRequested(navigation);
    });
}

void RelatedEvidencePanel::setActiveEvent(const scope::workspace::TimelineEvent* event,
                                            const std::vector<scope::workspace::EvidenceLinkRecord>& links,
                                            const std::vector<scope::workspace::TimelineEvent>& timelineEvents)
{
    m_activeEvent = event;
    m_links = links;
    m_timelineEvents = timelineEvents;
    setVisible(event != nullptr);
    m_addButton->setVisible(event != nullptr && !m_createMode);
    renderLinks();
}

void RelatedEvidencePanel::clear()
{
    m_activeEvent = nullptr;
    m_links.clear();
    m_timelineEvents.clear();
    m_list->clear();
    m_createForm->setVisible(false);
    m_createMode = false;
    m_addButton->setVisible(false);
    m_emptyLabel->setVisible(true);
    setVisible(false);
}

int RelatedEvidencePanel::rowCount() const
{
    return m_list->count();
}

bool RelatedEvidencePanel::openFirstRow()
{
    if (m_list->count() == 0)
    {
        return false;
    }

    QListWidgetItem* item = m_list->item(0);
    m_list->setCurrentItem(item);
    m_list->itemClicked(item);

    return true;
}

const scope::workspace::TimelineEvent* RelatedEvidencePanel::findEvent(const std::string& eventId) const
{
    for (const scope::workspace::TimelineEvent& event : m_timelineEvents)
    {
        if (event.id == eventId)
        {
            return &event;
        }
    }

    return nullptr;
}

void RelatedEvidencePanel::renderLinks()
{
    m_list->clear();

    if (m_activeEvent == nullptr)
    {
        m_emptyLabel->setVisible(true);

        return;
    }

    const QString activeId = QString::fromStdString(m_activeEvent->id);
    bool hasRows = false;

    for (const scope::workspace::EvidenceLinkRecord& link : m_links)
    {
        const QString sourceId = QString::fromStdString(link.source.eventId);
        const QString targetId = QString::fromStdString(link.target.eventId);

        if (sourceId != activeId && targetId != activeId)
        {
            continue;
        }

        const QString peerId = sourceId == activeId ? targetId : sourceId;
        const scope::workspace::TimelineEvent* peer = findEvent(peerId.toStdString());

        auto* item = new QListWidgetItem(m_list);
        item->setData(Qt::UserRole + 10, QStringLiteral("related-evidence-row"));
        item->setData(Qt::UserRole, QString::fromStdString(link.id));
        item->setData(Qt::UserRole + 1, peerId);

        QString label = linkTypeLabel(link.type);

        if (peer != nullptr)
        {
            label += QStringLiteral(" · %1 · %2")
                          .arg(QString::fromStdString(peer->timestamp),
                               QString::fromStdString(peer->source.artifactName));
            item->setData(Qt::UserRole + 2, QString::fromStdString(peer->source.artifactId));

            if (peer->source.lineNumber.has_value())
            {
                item->setData(Qt::UserRole + 3, static_cast<qulonglong>(*peer->source.lineNumber));
            }
        }

        if (link.status == scope::workspace::EvidenceLinkStatus::Stale || peer == nullptr)
        {
            item->setForeground(Qt::gray);
        }

        item->setText(label);
        m_list->addItem(item);
        hasRows = true;
    }

    m_emptyLabel->setVisible(!hasRows);
}

void RelatedEvidencePanel::populateTargetChoices()
{
    m_targetCombo->clear();

    if (m_activeEvent == nullptr)
    {
        return;
    }

    const QString activeId = QString::fromStdString(m_activeEvent->id);

    for (const scope::workspace::TimelineEvent& event : m_timelineEvents)
    {
        const QString eventId = QString::fromStdString(event.id);

        if (eventId == activeId)
        {
            continue;
        }

        const QString label = QString::fromStdString(event.timestamp) + QStringLiteral(" · ")
                              + QString::fromStdString(event.source.artifactName) + QStringLiteral(" · ")
                              + QString::fromStdString(event.message);

        m_targetCombo->addItem(label, eventId);
    }
}

} // namespace scope::desktop
