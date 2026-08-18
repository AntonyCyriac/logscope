/**
 * @file crash_panel.cpp
 */

#include "crash_panel.hpp"

#include "crash_load_worker.hpp"
#include "pstack_text_utils.hpp"

#include <QComboBox>
#include <QLabel>
#include <QListWidget>
#include <QTextBrowser>
#include <QThread>
#include <QVBoxLayout>

#include <QApplication>
#include <QElapsedTimer>

namespace scope::desktop
{

CrashPanel::CrashPanel(scope::application::InvestigationController* controller, QWidget* parent)
    : QWidget(parent), m_controller(controller)
{
    setupUi();

    m_workerThread = new QThread(this);
    m_worker = new CrashLoadWorker();
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_worker, &CrashLoadWorker::loadFinished, this, &CrashPanel::onLoadFinished);
    connect(m_worker, &CrashLoadWorker::loadFailed, this, &CrashPanel::onLoadFailed);

    m_workerThread->start();
}

CrashPanel::~CrashPanel()
{
    if (m_workerThread != nullptr)
    {
        m_workerThread->quit();
        m_workerThread->wait(2000);
    }
}

void CrashPanel::setupUi()
{
    auto* layout = new QVBoxLayout(this);

    m_errorLabel = new QLabel(this);
    m_errorLabel->setVisible(false);
    m_errorLabel->setWordWrap(true);

    m_emptyLabel = new QLabel(QStringLiteral("Open an investigation with a pstack or core artifact."), this);
    m_emptyLabel->setObjectName(QStringLiteral("crash-empty"));

    m_artifactCombo = new QComboBox(this);
    m_artifactCombo->setObjectName(QStringLiteral("crash-artifact-combo"));

    m_signalLabel = new QLabel(this);
    m_signalLabel->setObjectName(QStringLiteral("crash-signal"));

    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setWordWrap(true);

    m_threadList = new QListWidget(this);
    m_threadList->setObjectName(QStringLiteral("crash-thread-list"));

    m_pstackViewer = new QTextBrowser(this);
    m_pstackViewer->setObjectName(QStringLiteral("pstack-viewer"));

    layout->addWidget(m_errorLabel);
    layout->addWidget(m_emptyLabel);
    layout->addWidget(m_artifactCombo);
    layout->addWidget(m_signalLabel);
    layout->addWidget(m_summaryLabel);
    layout->addWidget(m_threadList);
    layout->addWidget(m_pstackViewer, 1);

    connect(m_artifactCombo, &QComboBox::currentIndexChanged, this, [this]() {
        m_activeArtifactId = currentArtifactId();
        refresh();
    });
    connect(m_threadList, &QListWidget::itemActivated, this, &CrashPanel::handleThreadActivated);
}

void CrashPanel::setInvestigationActive(const bool active)
{
    m_artifactCombo->setVisible(active);
    m_signalLabel->setVisible(active);
    m_summaryLabel->setVisible(active);
    m_threadList->setVisible(active);
    m_pstackViewer->setVisible(active);
    m_emptyLabel->setVisible(!active);

    if (!active)
    {
        m_artifactCombo->clear();
        m_signalLabel->clear();
        m_summaryLabel->clear();
        m_threadList->clear();
        m_pstackViewer->clear();
        m_report = {};
        m_pstackBody.clear();
        m_activeArtifactId.clear();
    }
}

QString CrashPanel::currentArtifactId() const
{
    return m_artifactCombo->currentData().toString();
}

void CrashPanel::populateArtifactChoices()
{
    m_artifactCombo->blockSignals(true);
    m_artifactCombo->clear();

    if (m_controller == nullptr || !m_controller->isOpen())
    {
        return;
    }

    for (const scope::workspace::ArtifactRecord& artifact : m_controller->manifest().artifacts)
    {
        if (artifact.type != "pstack" && artifact.type != "core")
        {
            continue;
        }

        m_artifactCombo->addItem(QString::fromStdString(artifact.name),
                                 QString::fromStdString(artifact.id));
    }

    if (m_activeArtifactId.isEmpty() && m_artifactCombo->count() > 0)
    {
        m_activeArtifactId = m_artifactCombo->itemData(0).toString();
        m_artifactCombo->setCurrentIndex(0);
    }
    else if (!m_activeArtifactId.isEmpty())
    {
        const int index = m_artifactCombo->findData(m_activeArtifactId);

        if (index >= 0)
        {
            m_artifactCombo->setCurrentIndex(index);
        }
    }

    m_artifactCombo->blockSignals(false);
}

void CrashPanel::setActiveArtifactId(const QString& artifactId)
{
    m_activeArtifactId = artifactId;
    populateArtifactChoices();
}

void CrashPanel::refresh()
{
    if (m_controller == nullptr || !m_controller->isOpen())
    {
        return;
    }

    populateArtifactChoices();

    const QString artifactId = currentArtifactId();

    if (artifactId.isEmpty())
    {
        m_emptyLabel->setVisible(true);
        m_emptyLabel->setText(QStringLiteral("Add a pstack or core artifact to analyze a crash."));

        return;
    }

    startAsyncLoad();
}

void CrashPanel::startAsyncLoad()
{
    if (m_loading)
    {
        m_refreshPending = true;

        return;
    }

    const QString artifactId = currentArtifactId();

    if (artifactId.isEmpty())
    {
        return;
    }

    m_loading = true;
    m_errorLabel->setVisible(false);
    m_emptyLabel->setVisible(false);

    const QString investigationDir = QString::fromStdString(m_controller->investigationDirectory().string());

    QMetaObject::invokeMethod(m_worker, "load", Qt::QueuedConnection, Q_ARG(QString, investigationDir),
                              Q_ARG(QString, artifactId));
}

void CrashPanel::onLoadFinished(const scope::workspace::CrashReport report)
{
    m_loading = false;
    m_report = report;
    renderReport(m_report);

    if (m_refreshPending)
    {
        m_refreshPending = false;
        startAsyncLoad();
    }
}

void CrashPanel::onLoadFailed(const QString& message)
{
    m_loading = false;
    m_refreshPending = false;
    m_errorLabel->setText(message);
    m_errorLabel->setVisible(true);
}

bool CrashPanel::waitForLoadComplete(const int timeoutMs)
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

void CrashPanel::renderReport(const scope::workspace::CrashReport& report)
{
    m_signalLabel->setText(report.signal.has_value() ? QString::fromStdString(*report.signal) : QString{});

    m_summaryLabel->setText(QString::fromStdString(report.summary));

    m_threadList->clear();

    for (const scope::workspace::CrashThread& thread : report.threads)
    {
        auto* item = new QListWidgetItem(QString::fromStdString(thread.name.empty() ? thread.id : thread.name));
        item->setData(Qt::UserRole, QString::fromStdString(thread.id));

        if (thread.isFaultThread)
        {
            item->setData(Qt::UserRole + 1, QStringLiteral("fault"));
        }

        m_threadList->addItem(item);
    }

    if (report.artifactType == "pstack")
    {
        const auto textResult = m_controller->readArtifactText(report.artifactId);

        if (textResult)
        {
            m_pstackBody = QString::fromStdString(*textResult);
            const QString highlightId =
                report.faultThreadId.has_value() ? QString::fromStdString(*report.faultThreadId) : QString{};

            renderPstackViewer(m_pstackBody, highlightId);
        }
    }
    else
    {
        m_pstackViewer->clear();
    }
}

void CrashPanel::renderPstackViewer(const QString& body, const QString& highlightThreadId)
{
    m_highlightThreadId = highlightThreadId;
    m_pstackViewer->clear();

    const std::vector<PstackThreadBlock> blocks = splitPstackThreads(body);

    if (blocks.empty())
    {
        m_pstackViewer->setPlainText(body);

        return;
    }

    QString html;

    for (const PstackThreadBlock& block : blocks)
    {
        const bool highlight = !highlightThreadId.isEmpty() && block.id == highlightThreadId;
        const QString escaped = QString(block.text).toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br>"));
        const QString cssClass = highlight ? QStringLiteral("crash-pstack-thread--highlight") : QStringLiteral("crash-pstack-thread");

        html += QStringLiteral("<pre class=\"%1\" data-testid=\"pstack-thread\" data-thread-id=\"%2\">%3</pre>")
                    .arg(cssClass, block.id.toHtmlEscaped(), escaped);
    }

    m_pstackViewer->setHtml(html);
}

void CrashPanel::handleThreadActivated(QListWidgetItem* item)
{
    if (item == nullptr || m_controller == nullptr)
    {
        return;
    }

    const QString threadId = item->data(Qt::UserRole).toString();

    renderPstackViewer(m_pstackBody, threadId);

    ViewerNavigation navigation;
    navigation.artifactId = currentArtifactId();
    navigation.faultThreadId = threadId;
    navigation.targetTab = QStringLiteral("crash");
    navigation.statusMessage = QStringLiteral("Jumped to pstack thread %1").arg(threadId);

    emit navigationRequested(navigation);
}

QString CrashPanel::signalText() const
{
    return m_signalLabel->text();
}

bool CrashPanel::clickFaultThread()
{
    for (int row = 0; row < m_threadList->count(); ++row)
    {
        QListWidgetItem* item = m_threadList->item(row);

        if (item == nullptr)
        {
            continue;
        }

        if (item->data(Qt::UserRole + 1).toString() == QStringLiteral("fault"))
        {
            m_threadList->setCurrentItem(item);
            handleThreadActivated(item);

            return true;
        }
    }

    if (m_threadList->count() > 0)
    {
        QListWidgetItem* item = m_threadList->item(0);
        m_threadList->setCurrentItem(item);
        handleThreadActivated(item);

        return true;
    }

    return false;
}

} // namespace scope::desktop
