/**
 * @file timeline_panel.cpp
 */

#include "timeline_panel.hpp"

#include <QColor>
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

} // namespace

TimelinePanel::TimelinePanel(scope::application::InvestigationController* controller, QWidget* parent)
    : QWidget(parent), m_controller(controller)
{
    setupUi();
}

TimelinePanel::~TimelinePanel() = default;

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
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels(
        {QStringLiteral("Timestamp"), QStringLiteral("Type"), QStringLiteral("Artifact"),
         QStringLiteral("Message"), QStringLiteral("Severity")});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setVisible(false);

    m_loadMoreButton = new QPushButton(QStringLiteral("Load more"), this);
    m_loadMoreButton->setObjectName(QStringLiteral("timeline-load-more"));
    m_loadMoreButton->setVisible(false);

    layout->addWidget(m_errorLabel);
    layout->addWidget(m_emptyLabel);
    layout->addWidget(m_table, 1);
    layout->addWidget(m_loadMoreButton);

    connect(m_table, &QTableWidget::cellClicked, this, &TimelinePanel::handleRowActivated);
    connect(m_loadMoreButton, &QPushButton::clicked, this, [this]() {
        refresh();
    });
}

void TimelinePanel::setInvestigationActive(const bool active)
{
    m_emptyLabel->setText(active ? QStringLiteral("Loading timeline…")
                                 : QStringLiteral("Open an investigation to view its timeline."));
    m_emptyLabel->setVisible(!active || m_events.empty());
    m_table->setVisible(active && !m_events.empty());

    if (!active)
    {
        clearTable();
    }
}

void TimelinePanel::clearTable()
{
    m_events.clear();
    m_offset = 0U;
    m_truncated = false;
    m_table->setRowCount(0);
    m_errorLabel->clear();
    m_errorLabel->setVisible(false);
    m_loadMoreButton->setVisible(false);
}

void TimelinePanel::refresh()
{
    if (m_controller == nullptr || !m_controller->isOpen() || m_loading)
    {
        return;
    }

    if (m_offset == 0U)
    {
        clearTable();
    }

    m_loading = true;
    m_errorLabel->setVisible(false);
    m_emptyLabel->setVisible(false);

    scope::workspace::TimelineProjectionOptions options;
    options.limit = kTimelinePageSize;
    options.offset = m_offset;

    const auto timelineResult = m_controller->projectTimeline(options);

    m_loading = false;

    if (!timelineResult)
    {
        m_errorLabel->setText(QString::fromStdString(timelineResult.error().message()));
        m_errorLabel->setVisible(true);

        return;
    }

    if (m_offset == 0U)
    {
        m_events.clear();
    }

    m_events.insert(m_events.end(), timelineResult->events.begin(), timelineResult->events.end());
    m_truncated = timelineResult->truncated;
    m_offset = m_events.size();
    m_loadMoreButton->setVisible(m_truncated);
    populateTable(*timelineResult);
}

QString TimelinePanel::formatEventType(const scope::workspace::TimelineEvent& event)
{
    if (event.eventType == "crash.summary")
    {
        return QStringLiteral("Crash");
    }

    return QString::fromStdString(event.eventType);
}

void TimelinePanel::populateTable(const scope::workspace::TimelineProjectionResult& result)
{
    Q_UNUSED(result);

    m_table->setRowCount(static_cast<int>(m_events.size()));
    m_table->setVisible(!m_events.empty());
    m_emptyLabel->setVisible(m_events.empty());

    for (int row = 0; row < static_cast<int>(m_events.size()); ++row)
    {
        const scope::workspace::TimelineEvent& event = m_events[static_cast<std::size_t>(row)];

        m_table->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(event.timestamp)));
        m_table->setItem(row, 1, new QTableWidgetItem(formatEventType(event)));
        m_table->setItem(row, 2,
                         new QTableWidgetItem(QString::fromStdString(event.source.artifactName)));
        m_table->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(event.message)));

        const QString severity =
            event.severity.has_value() ? QString::fromStdString(*event.severity) : QString{};

        m_table->setItem(row, 4, new QTableWidgetItem(severity));

        for (int column = 0; column < m_table->columnCount(); ++column)
        {
            QTableWidgetItem* item = m_table->item(row, column);

            if (item != nullptr)
            {
                item->setData(Qt::UserRole, QString::fromStdString(event.id));
                item->setData(Qt::UserRole + 1, QString::fromStdString(event.eventType));
                item->setData(Qt::UserRole + 2, QString::fromStdString(event.artifactId));

                if (event.eventType == "crash.summary")
                {
                    item->setBackground(QColor(80, 32, 32));
                }
            }
        }

        m_table->setProperty("testId", QStringLiteral("timeline-row"));
    }
}

void TimelinePanel::handleRowActivated(const int row)
{
    if (row < 0 || row >= static_cast<int>(m_events.size()))
    {
        return;
    }

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

    emit navigationRequested(navigation);
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

} // namespace scope::desktop
