/**
 * @file timeline_panel.cpp
 */

#include "timeline_panel.hpp"

#include "related_evidence_panel.hpp"
#include "suggested_connections_panel.hpp"
#include "timeline_load_worker.hpp"

#include <QApplication>
#include <QColor>
#include <QElapsedTimer>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QThread>
#include <QVBoxLayout>

namespace scope::desktop
{

namespace
{

constexpr int kTimelinePageSize = 500;

scope::workspace::EvidenceLinkType parseLinkType(const QString& value)
{
    if (value == QStringLiteral("PRECEDES"))
    {
        return scope::workspace::EvidenceLinkType::Precedes;
    }

    if (value == QStringLiteral("FOLLOWS"))
    {
        return scope::workspace::EvidenceLinkType::Follows;
    }

    if (value == QStringLiteral("SUPPORTS"))
    {
        return scope::workspace::EvidenceLinkType::Supports;
    }

    return scope::workspace::EvidenceLinkType::Related;
}

} // namespace

TimelinePanel::TimelinePanel(scope::application::InvestigationController* controller, QWidget* parent)
    : QWidget(parent), m_controller(controller)
{
    setupUi();

    m_workerThread = new QThread(this);
    m_worker = new TimelineLoadWorker();
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_worker, &TimelineLoadWorker::loadFinished, this, &TimelinePanel::onLoadFinished);
    connect(m_worker, &TimelineLoadWorker::loadFailed, this, &TimelinePanel::onLoadFailed);

    m_workerThread->start();
}

TimelinePanel::~TimelinePanel()
{
    if (m_workerThread != nullptr)
    {
        m_workerThread->quit();
        m_workerThread->wait(2000);
    }
}

void TimelinePanel::setupUi()
{
    auto* layout = new QVBoxLayout(this);

    m_errorLabel = new QLabel(this);
    m_errorLabel->setObjectName(QStringLiteral("timeline-error"));
    m_errorLabel->setVisible(false);
    m_errorLabel->setWordWrap(true);

    m_emptyLabel = new QLabel(QStringLiteral("Open an investigation to view its timeline."), this);
    m_emptyLabel->setObjectName(QStringLiteral("timeline-empty"));

    m_table = new QTableWidget(this);
    m_table->setObjectName(QStringLiteral("timeline-table"));
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({QStringLiteral("Timestamp"), QStringLiteral("Type"),
                                          QStringLiteral("Artifact"), QStringLiteral("Message"),
                                          QStringLiteral("Severity"), QStringLiteral("Connections")});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setVisible(false);

    m_loadMoreButton = new QPushButton(QStringLiteral("Load more"), this);
    m_loadMoreButton->setObjectName(QStringLiteral("timeline-load-more"));
    m_loadMoreButton->setVisible(false);

    m_suggestionsPanel = new SuggestedConnectionsPanel(this);
    m_relatedPanel = new RelatedEvidencePanel(this);

    layout->addWidget(m_errorLabel);
    layout->addWidget(m_emptyLabel);
    layout->addWidget(m_table, 1);
    layout->addWidget(m_loadMoreButton);
    layout->addWidget(m_suggestionsPanel);
    layout->addWidget(m_relatedPanel);

    connect(m_table, &QTableWidget::cellClicked, this, &TimelinePanel::handleRowActivated);
    connect(m_loadMoreButton, &QPushButton::clicked, this, [this]() {
        refresh();
    });

    connect(m_relatedPanel, &RelatedEvidencePanel::createLinkRequested, this,
            &TimelinePanel::handleCreateLink);
    connect(m_relatedPanel, &RelatedEvidencePanel::removeLinkRequested, this, &TimelinePanel::handleRemoveLink);
    connect(m_relatedPanel, &RelatedEvidencePanel::openLinkRequested, this,
            &TimelinePanel::navigationRequested);
    connect(m_suggestionsPanel, &SuggestedConnectionsPanel::acceptRequested, this,
            &TimelinePanel::handleAcceptSuggestion);
    connect(m_suggestionsPanel, &SuggestedConnectionsPanel::dismissRequested, this,
            &TimelinePanel::handleDismissSuggestion);
}

void TimelinePanel::setDismissedSuggestionIds(std::unordered_set<std::string>* dismissedSuggestionIds)
{
    m_dismissedSuggestionIds = dismissedSuggestionIds;
}

void TimelinePanel::setInvestigationActive(const bool active)
{
    m_emptyLabel->setText(active ? QStringLiteral("Loading timeline…")
                                 : QStringLiteral("Open an investigation to view its timeline."));
    m_emptyLabel->setVisible(!active || (m_events.empty() && !m_loading));
    m_table->setVisible(active && !m_events.empty());

    if (!active)
    {
        clearTable();
        m_relatedPanel->clear();
        m_suggestionsPanel->clear();
    }
}

void TimelinePanel::clearTable()
{
    m_events.clear();
    m_allLinks.clear();
    m_offset = 0U;
    m_truncated = false;
    m_selectedRow = -1;
    m_table->setRowCount(0);
    m_errorLabel->clear();
    m_errorLabel->setVisible(false);
    m_loadMoreButton->setVisible(false);
}

void TimelinePanel::refresh()
{
    if (m_controller == nullptr || !m_controller->isOpen())
    {
        return;
    }

    if (m_offset == 0U)
    {
        clearTable();
    }

    startAsyncLoad();
}

void TimelinePanel::startAsyncLoad()
{
    if (m_loading)
    {
        m_refreshPending = true;

        return;
    }

    m_loading = true;
    m_errorLabel->setVisible(false);
    m_emptyLabel->setText(QStringLiteral("Loading timeline…"));
    m_emptyLabel->setVisible(true);

    const QString investigationDir = QString::fromStdString(m_controller->investigationDirectory().string());

    QMetaObject::invokeMethod(m_worker, "load", Qt::QueuedConnection, Q_ARG(QString, investigationDir),
                              Q_ARG(quint64, static_cast<quint64>(kTimelinePageSize)),
                              Q_ARG(quint64, static_cast<quint64>(m_offset)));
}

void TimelinePanel::onLoadFinished(const scope::workspace::TimelineProjectionResult result)
{
    m_loading = false;

    if (m_offset == 0U)
    {
        m_events.clear();
    }

    m_events.insert(m_events.end(), result.events.begin(), result.events.end());
    m_truncated = result.truncated;
    m_offset = m_events.size();
    m_loadMoreButton->setVisible(m_truncated);

    const auto linksResult = m_controller->listEvidenceLinks();

    if (linksResult)
    {
        m_allLinks = *linksResult;
    }

    populateTable();

    if (m_refreshPending)
    {
        m_refreshPending = false;
        startAsyncLoad();
    }
}

void TimelinePanel::onLoadFailed(const QString& message)
{
    m_loading = false;
    m_refreshPending = false;
    m_errorLabel->setText(message);
    m_errorLabel->setVisible(true);
}

bool TimelinePanel::waitForLoadComplete(const int timeoutMs)
{
    QElapsedTimer timer;
    timer.start();

    while (m_loading && timer.elapsed() < timeoutMs)
    {
        QApplication::processEvents();
        QThread::msleep(10);
    }

    return !m_loading;
}

QString TimelinePanel::formatEventType(const scope::workspace::TimelineEvent& event)
{
    if (event.eventType == "crash.summary")
    {
        return QStringLiteral("Crash");
    }

    return QString::fromStdString(event.eventType);
}

int TimelinePanel::linkCountForEvent(const std::string& eventId) const
{
    int count = 0;

    for (const scope::workspace::EvidenceLinkRecord& link : m_allLinks)
    {
        if (link.source.eventId == eventId || link.target.eventId == eventId)
        {
            ++count;
        }
    }

    return count;
}

void TimelinePanel::populateTable()
{
    m_table->setRowCount(static_cast<int>(m_events.size()));
    m_table->setVisible(!m_events.empty());
    m_emptyLabel->setVisible(m_events.empty() && !m_loading);

    for (int row = 0; row < static_cast<int>(m_events.size()); ++row)
    {
        const scope::workspace::TimelineEvent& event = m_events[static_cast<std::size_t>(row)];
        const int linkCount = linkCountForEvent(event.id);

        m_table->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(event.timestamp)));
        m_table->setItem(row, 1, new QTableWidgetItem(formatEventType(event)));
        m_table->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(event.source.artifactName)));
        m_table->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(event.message)));

        const QString severity =
            event.severity.has_value() ? QString::fromStdString(*event.severity) : QString{};

        m_table->setItem(row, 4, new QTableWidgetItem(severity));

        QString connections;

        if (linkCount > 0)
        {
            connections = QStringLiteral("Related (%1)").arg(linkCount);
        }

        auto* connectionsItem = new QTableWidgetItem(connections);
        connectionsItem->setData(Qt::UserRole + 10, QStringLiteral("timeline-link-badge"));
        m_table->setItem(row, 5, connectionsItem);

        for (int column = 0; column < m_table->columnCount(); ++column)
        {
            QTableWidgetItem* item = m_table->item(row, column);

            if (item != nullptr)
            {
                item->setData(Qt::UserRole, QString::fromStdString(event.id));
                item->setData(Qt::UserRole + 1, QString::fromStdString(event.eventType));
                item->setData(Qt::UserRole + 2, QString::fromStdString(event.artifactId));
                item->setData(Qt::UserRole + 11, QStringLiteral("timeline-row"));

                if (event.eventType == "crash.summary")
                {
                    item->setBackground(QColor(80, 32, 32));
                }
            }
        }
    }
}

void TimelinePanel::handleRowActivated(const int row)
{
    if (row < 0 || row >= static_cast<int>(m_events.size()))
    {
        return;
    }

    m_selectedRow = row;
    m_table->selectRow(row);

    const scope::workspace::TimelineEvent& event = m_events[static_cast<std::size_t>(row)];
    ViewerNavigation navigation;

    if (event.eventType == "crash.summary")
    {
        navigation.artifactId = QString::fromStdString(event.artifactId);
        navigation.targetTab = QStringLiteral("crash");
        navigation.statusMessage = QString::fromStdString(event.message);
    }
    else if (event.eventType == "log.line")
    {
        navigation.artifactId = QString::fromStdString(event.source.artifactId);
        navigation.targetTab = QStringLiteral("results");

        if (event.source.lineNumber.has_value())
        {
            navigation.lineNumber = *event.source.lineNumber;
        }

        navigation.statusMessage =
            QStringLiteral("Timeline: %1").arg(QString::fromStdString(event.message));
    }
    else
    {
        navigation.targetTab = QStringLiteral("timeline");
        navigation.statusMessage = QString::fromStdString(event.message);
    }

    refreshPanelsForSelection();

    if (event.eventType == "crash.summary")
    {
        emit navigationRequested(navigation);
    }
    else if (event.eventType == "log.line")
    {
        emit statusMessageRequested(navigation.statusMessage);
    }
    else if (!navigation.targetTab.isEmpty() || !navigation.statusMessage.isEmpty())
    {
        emit navigationRequested(navigation);
    }
}

std::vector<scope::workspace::EvidenceLinkRecord> TimelinePanel::linksForActiveEvent() const
{
    if (m_selectedRow < 0 || m_selectedRow >= static_cast<int>(m_events.size()))
    {
        return {};
    }

    const std::string activeId = m_events[static_cast<std::size_t>(m_selectedRow)].id;
    std::vector<scope::workspace::EvidenceLinkRecord> filtered;

    for (const scope::workspace::EvidenceLinkRecord& link : m_allLinks)
    {
        if (link.source.eventId == activeId || link.target.eventId == activeId)
        {
            filtered.push_back(link);
        }
    }

    return filtered;
}

void TimelinePanel::refreshPanelsForSelection()
{
    if (m_selectedRow < 0 || m_selectedRow >= static_cast<int>(m_events.size()))
    {
        m_relatedPanel->clear();
        m_suggestionsPanel->clear();

        return;
    }

    const scope::workspace::TimelineEvent& event = m_events[static_cast<std::size_t>(m_selectedRow)];
    m_relatedPanel->setActiveEvent(&event, linksForActiveEvent(), m_events);

    scope::workspace::CorrelationSuggestionQuery query;
    query.eventId = event.id;

    const std::unordered_set<std::string> dismissed =
        m_dismissedSuggestionIds != nullptr ? *m_dismissedSuggestionIds : std::unordered_set<std::string>{};

    const auto suggestionsResult = m_controller->listCorrelationSuggestions(query, dismissed);

    if (suggestionsResult && suggestionsResult->total > 0)
    {
        m_suggestionsPanel->setSuggestions(suggestionsResult->suggestions);
    }
    else
    {
        m_suggestionsPanel->clear();
    }
}

void TimelinePanel::handleCreateLink(const QString& targetEventId, const QString& linkType, const QString& note)
{
    if (m_selectedRow < 0 || m_selectedRow >= static_cast<int>(m_events.size()))
    {
        return;
    }

    scope::workspace::EvidenceLinkCreateRequest request;
    request.type = parseLinkType(linkType);
    request.source.kind = "timeline_event";
    request.source.eventId = m_events[static_cast<std::size_t>(m_selectedRow)].id;
    request.target.kind = "timeline_event";
    request.target.eventId = targetEventId.toStdString();

    if (!note.isEmpty())
    {
        request.note = note.toStdString();
    }

    const auto result = m_controller->addEvidenceLink(request);

    if (!result)
    {
        emit statusMessageRequested(QString::fromStdString(result.error().message()));

        return;
    }

    emit statusMessageRequested(QStringLiteral("Connection added"));

    const auto linksResult = m_controller->listEvidenceLinks();

    if (linksResult)
    {
        m_allLinks = *linksResult;
    }

    populateTable();
    refreshPanelsForSelection();
}

void TimelinePanel::handleRemoveLink(const QString& linkId)
{
    const auto result = m_controller->removeEvidenceLink(linkId.toStdString());

    if (!result)
    {
        emit statusMessageRequested(QString::fromStdString(result.error().message()));

        return;
    }

    emit statusMessageRequested(QStringLiteral("Connection removed"));

    const auto linksResult = m_controller->listEvidenceLinks();

    if (linksResult)
    {
        m_allLinks = *linksResult;
    }

    populateTable();
    refreshPanelsForSelection();
}

void TimelinePanel::handleAcceptSuggestion(const QString& suggestionId)
{
    const std::unordered_set<std::string> dismissed =
        m_dismissedSuggestionIds != nullptr ? *m_dismissedSuggestionIds : std::unordered_set<std::string>{};

    const auto result = m_controller->acceptCorrelationSuggestion(suggestionId.toStdString(), dismissed);

    if (!result)
    {
        emit statusMessageRequested(QString::fromStdString(result.error().message()));

        return;
    }

    emit statusMessageRequested(QStringLiteral("Connection added from suggestion"));

    const auto linksResult = m_controller->listEvidenceLinks();

    if (linksResult)
    {
        m_allLinks = *linksResult;
    }

    populateTable();
    refreshPanelsForSelection();
}

void TimelinePanel::handleDismissSuggestion(const QString& suggestionId)
{
    if (m_dismissedSuggestionIds != nullptr)
    {
        m_dismissedSuggestionIds->insert(suggestionId.toStdString());
    }

    emit statusMessageRequested(QStringLiteral("Suggestion dismissed"));
    refreshPanelsForSelection();
}

QString TimelinePanel::rowMessage(const int row) const
{
    if (row < 0 || row >= static_cast<int>(m_events.size()))
    {
        return {};
    }

    return QString::fromStdString(m_events[static_cast<std::size_t>(row)].message);
}

int TimelinePanel::rowCount() const
{
    return static_cast<int>(m_events.size());
}

bool TimelinePanel::selectRowByEventType(const QString& eventType)
{
    for (int row = 0; row < static_cast<int>(m_events.size()); ++row)
    {
        if (QString::fromStdString(m_events[static_cast<std::size_t>(row)].eventType) == eventType)
        {
            m_table->selectRow(row);
            handleRowActivated(row);

            return true;
        }
    }

    return false;
}

bool TimelinePanel::selectRow(const int row)
{
    if (row < 0 || row >= static_cast<int>(m_events.size()))
    {
        return false;
    }

    m_table->selectRow(row);
    handleRowActivated(row);

    return true;
}

bool TimelinePanel::hasEventType(const QString& eventType) const
{
    for (const scope::workspace::TimelineEvent& event : m_events)
    {
        if (QString::fromStdString(event.eventType) == eventType)
        {
            return true;
        }
    }

    return false;
}

int TimelinePanel::linkBadgeCount() const
{
    int count = 0;

    for (int row = 0; row < m_table->rowCount(); ++row)
    {
        QTableWidgetItem* item = m_table->item(row, 5);

        if (item != nullptr
            && item->data(Qt::UserRole + 10).toString() == QStringLiteral("timeline-link-badge")
            && !item->text().isEmpty())
        {
            ++count;
        }
    }

    return count;
}

int TimelinePanel::relatedEvidenceRowCount() const
{
    return m_relatedPanel->rowCount();
}

bool TimelinePanel::suggestionPanelVisible() const
{
    return m_suggestionsPanel->isVisibleForTests();
}

bool TimelinePanel::createLinkBetweenRows(const int sourceRow, const int targetRow)
{
    if (sourceRow < 0 || targetRow < 0 || sourceRow >= static_cast<int>(m_events.size())
        || targetRow >= static_cast<int>(m_events.size()))
    {
        return false;
    }

    m_selectedRow = sourceRow;

    handleCreateLink(QString::fromStdString(m_events[static_cast<std::size_t>(targetRow)].id),
                     QStringLiteral("RELATED"), QString{});

    return linkCountForEvent(m_events[static_cast<std::size_t>(sourceRow)].id) > 0;
}

bool TimelinePanel::removeFirstLinkForSelection()
{
    const auto links = linksForActiveEvent();

    if (links.empty())
    {
        return false;
    }

    handleRemoveLink(QString::fromStdString(links.front().id));

    return true;
}

bool TimelinePanel::removeFirstEvidenceLink()
{
    return removeFirstLinkForSelection();
}

bool TimelinePanel::openFirstRelatedEvidence()
{
    return m_relatedPanel->openFirstRow();
}

bool TimelinePanel::acceptFirstSuggestion()
{
    if (m_selectedRow < 0)
    {
        return false;
    }

    scope::workspace::CorrelationSuggestionQuery query;
    query.eventId = m_events[static_cast<std::size_t>(m_selectedRow)].id;

    const std::unordered_set<std::string> dismissed =
        m_dismissedSuggestionIds != nullptr ? *m_dismissedSuggestionIds : std::unordered_set<std::string>{};

    const auto suggestionsResult = m_controller->listCorrelationSuggestions(query, dismissed);

    if (!suggestionsResult || suggestionsResult->suggestions.empty())
    {
        return false;
    }

    handleAcceptSuggestion(QString::fromStdString(suggestionsResult->suggestions.front().id));

    return relatedEvidenceRowCount() > 0;
}

bool TimelinePanel::dismissFirstSuggestion()
{
    if (m_selectedRow < 0)
    {
        return false;
    }

    scope::workspace::CorrelationSuggestionQuery query;
    query.eventId = m_events[static_cast<std::size_t>(m_selectedRow)].id;

    const std::unordered_set<std::string> dismissed =
        m_dismissedSuggestionIds != nullptr ? *m_dismissedSuggestionIds : std::unordered_set<std::string>{};

    const auto suggestionsResult = m_controller->listCorrelationSuggestions(query, dismissed);

    if (!suggestionsResult || suggestionsResult->suggestions.empty())
    {
        return false;
    }

    const std::size_t linkCountBefore = m_allLinks.size();

    handleDismissSuggestion(QString::fromStdString(suggestionsResult->suggestions.front().id));

    return m_allLinks.size() == linkCountBefore && !suggestionPanelVisible();
}

} // namespace scope::desktop
