/**
 * @file main_window.cpp
 */

#include "main_window.hpp"

#include <QGuiApplication>
#include <QLabel>
#include <QHeaderView>
#include <QClipboard>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QMenuBar>
#include <QMessageBox>
#include <QInputDialog>
#include <QSplitter>
#include <QTabWidget>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <sstream>

#include "analysis_config.hpp"
#include "analytics_config.hpp"
#include "field_filter.hpp"
#include "foundation/path.hpp"
#include "foundation/timestamp.hpp"
#include "indexed_line_access.hpp"
#include "report_options.hpp"
#include "report_writer.hpp"
#include "theme_manager.hpp"
#include "time_range_filter.hpp"

namespace scope::desktop
{

namespace
{

QString extensionStatusLabel(const scope::extension::ExtensionStatus status)
{
    switch (status)
    {
    case scope::extension::ExtensionStatus::Ready:
        return QStringLiteral("ready");
    case scope::extension::ExtensionStatus::Disabled:
        return QStringLiteral("disabled");
    case scope::extension::ExtensionStatus::InitializationFailed:
        return QStringLiteral("failed");
    }

    return QStringLiteral("unknown");
}

scope::foundation::Path resolveInvestigationsRoot(const scope::foundation::Path& overrideRoot)
{
    if (!overrideRoot.string().empty())
    {
        return overrideRoot;
    }

    const QString root =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/investigations");

    return scope::foundation::Path(root.toStdString());
}

} // namespace

MainWindow::MainWindow(const scope::foundation::Path& configFile,
                       const scope::foundation::Path& investigationsRoot, QWidget* parent)
    : QMainWindow(parent), m_investigationController(resolveInvestigationsRoot(investigationsRoot))
{
    setWindowTitle(QStringLiteral("LogScope Desktop"));
    resize(1200, 800);

    ThemeManager::apply(ThemeMode::System);
    [[maybe_unused]] const auto configResult = m_service.loadConfiguration(configFile);

    createMenus();
    createLayout();

    m_tailWorker = new TailWorker(&m_service, this);
    connect(m_tailWorker, &TailWorker::linesAppended, this, [this](const QStringList& lines) {
        std::vector<std::string> raw;

        for (const QString& line : lines)
        {
            raw.push_back(line.toStdString());
        }

        m_logModel->appendRawLines(raw, 0U);
    });

    connect(m_tailWorker, &TailWorker::tailError, this, [this](const QString& message) {
        updateStatus(message);
    });

    updateStatus(QStringLiteral("Ready"));
}

void MainWindow::createMenus()
{
    auto* fileMenu = menuBar()->addMenu(QStringLiteral("File"));
    fileMenu->addAction(QStringLiteral("Open…"), this, &MainWindow::openFile);
    fileMenu->addAction(QStringLiteral("Open from Clipboard"), this, &MainWindow::openFromClipboard);
    fileMenu->addAction(QStringLiteral("Open Stdin"), this, &MainWindow::openStdin);
    fileMenu->addAction(QStringLiteral("Load Configuration…"), this, &MainWindow::loadConfigurationFile);
    fileMenu->addAction(QStringLiteral("Configuration…"), this, &MainWindow::showConfigurationEditor);
    fileMenu->addAction(QStringLiteral("Export Report…"), this, &MainWindow::exportReport);
    fileMenu->addSeparator();
    fileMenu->addAction(QStringLiteral("Quit"), this, &QWidget::close);

    auto* sessionMenu = menuBar()->addMenu(QStringLiteral("Session"));
    sessionMenu->addAction(QStringLiteral("Save…"), this, &MainWindow::saveSession);
    sessionMenu->addAction(QStringLiteral("Load…"), this, &MainWindow::loadSession);

    auto* investigationMenu = menuBar()->addMenu(QStringLiteral("Investigation"));
    investigationMenu->addAction(QStringLiteral("New Investigation…"), this, &MainWindow::promptCreateInvestigation);
    investigationMenu->addAction(QStringLiteral("Open Investigation…"), this, &MainWindow::openInvestigation);
    investigationMenu->addAction(QStringLiteral("Close Investigation"), this, &MainWindow::closeInvestigation);
    investigationMenu->addSeparator();
    investigationMenu->addAction(QStringLiteral("Add Artifact…"), this, &MainWindow::addInvestigationArtifact);

    auto* viewMenu = menuBar()->addMenu(QStringLiteral("View"));
    viewMenu->addAction(QStringLiteral("Run Statistics…"), this, &MainWindow::showRunStats);
    viewMenu->addAction(QStringLiteral("Light Theme"), this, &MainWindow::applyLightTheme);
    viewMenu->addAction(QStringLiteral("Dark Theme"), this, &MainWindow::applyDarkTheme);
}

void MainWindow::createLayout()
{
    auto* splitter = new QSplitter(this);
    setCentralWidget(splitter);

    auto* navigator = new QWidget(splitter);
    auto* navLayout = new QVBoxLayout(navigator);

    m_sessionList = new QListWidget(navigator);
    m_artifactList = new QListWidget(navigator);
    m_artifactList->setObjectName(QStringLiteral("artifact-list"));
    m_artifactList->setVisible(false);
    m_extensionList = new QListWidget(navigator);
    m_extensionDetails = new QTextEdit(navigator);
    m_extensionDetails->setReadOnly(true);
    m_extensionDetails->setPlaceholderText(QStringLiteral("Select an extension"));
    m_extensionDetails->setMaximumHeight(120);
    navLayout->addWidget(new QLabel(QStringLiteral("Workspace"), navigator));
    navLayout->addWidget(m_sessionList);
    navLayout->addWidget(new QLabel(QStringLiteral("Artifacts"), navigator));
    navLayout->addWidget(m_artifactList);
    navLayout->addWidget(new QLabel(QStringLiteral("Extensions"), navigator));
    navLayout->addWidget(m_extensionList);
    navLayout->addWidget(m_extensionDetails);

    auto* workArea = new QWidget(splitter);
    auto* workLayout = new QVBoxLayout(workArea);

    auto* toolbar = new QHBoxLayout();
    auto* openButton = new QPushButton(QStringLiteral("Open"), workArea);
    m_tailCheck = new QCheckBox(QStringLiteral("Tail"), workArea);
    m_persistIndexCheck = new QCheckBox(QStringLiteral("Persist index"), workArea);
    m_reuseIndexCheck = new QCheckBox(QStringLiteral("Reuse index"), workArea);
    auto* analyzeButton = new QPushButton(QStringLiteral("Analyze"), workArea);
    auto* investigateButton = new QPushButton(QStringLiteral("Investigate"), workArea);

    toolbar->addWidget(openButton);
    toolbar->addWidget(m_tailCheck);
    toolbar->addWidget(m_persistIndexCheck);
    toolbar->addWidget(m_reuseIndexCheck);
    toolbar->addWidget(analyzeButton);
    toolbar->addWidget(investigateButton);

    auto* filterRow = new QHBoxLayout();
    m_searchEdit = new QLineEdit(workArea);
    m_searchEdit->setPlaceholderText(QStringLiteral("Search"));
    m_queryEdit = new QLineEdit(workArea);
    m_queryEdit->setPlaceholderText(QStringLiteral("Query"));
    m_filterEdit = new QLineEdit(workArea);
    m_filterEdit->setPlaceholderText(QStringLiteral("Filter DSL"));
    m_levelEdit = new QLineEdit(workArea);
    m_levelEdit->setPlaceholderText(QStringLiteral("Level"));
    m_timeFromEdit = new QLineEdit(workArea);
    m_timeFromEdit->setPlaceholderText(QStringLiteral("From"));
    m_timeToEdit = new QLineEdit(workArea);
    m_timeToEdit->setPlaceholderText(QStringLiteral("To"));
    m_regexCheck = new QCheckBox(QStringLiteral("Regex"), workArea);
    m_caseCheck = new QCheckBox(QStringLiteral("Case"), workArea);

    filterRow->addWidget(m_searchEdit);
    filterRow->addWidget(m_queryEdit);
    filterRow->addWidget(m_filterEdit);
    filterRow->addWidget(m_levelEdit);
    filterRow->addWidget(m_timeFromEdit);
    filterRow->addWidget(m_timeToEdit);
    filterRow->addWidget(m_regexCheck);
    filterRow->addWidget(m_caseCheck);

    m_logModel = new LogTableModel(this);
    m_logView = new QTableView(workArea);
    m_logView->setModel(m_logModel);
    m_logView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_logView->horizontalHeader()->setStretchLastSection(true);

    auto* resultsTab = new QWidget(workArea);
    auto* resultsLayout = new QVBoxLayout(resultsTab);
    resultsLayout->setContentsMargins(0, 0, 0, 0);
    resultsLayout->addLayout(filterRow);
    resultsLayout->addWidget(m_logView, 1);

    m_bottomTabs = new QTabWidget(workArea);
    m_timelinePanel = new TimelinePanel(&m_investigationController, m_bottomTabs);
    m_crashPanel = new CrashPanel(&m_investigationController, m_bottomTabs);
    m_bottomTabs->addTab(m_timelinePanel, QStringLiteral("Timeline"));
    m_bottomTabs->addTab(m_crashPanel, QStringLiteral("Crash"));
    m_bottomTabs->addTab(resultsTab, QStringLiteral("Results"));
    m_analyticsPanel = new AnalyticsPanel(m_bottomTabs);
    m_aiPanel = new AiPanel(&m_service, m_bottomTabs);
    m_bottomTabs->addTab(m_aiPanel, QStringLiteral("AI"));
    m_bottomTabs->addTab(m_analyticsPanel, QStringLiteral("Analytics"));

    workLayout->addLayout(toolbar);
    workLayout->addWidget(m_bottomTabs, 1);

    splitter->addWidget(navigator);
    splitter->addWidget(workArea);
    splitter->setStretchFactor(1, 1);

    connect(openButton, &QPushButton::clicked, this, &MainWindow::openFile);
    connect(analyzeButton, &QPushButton::clicked, this, &MainWindow::runAnalyze);
    connect(investigateButton, &QPushButton::clicked, this, &MainWindow::runInvestigate);
    connect(m_bottomTabs, &QTabWidget::currentChanged, this, [this](const int index) {
        if (m_bottomTabs == nullptr)
        {
            return;
        }

        if (m_bottomTabs->widget(index) == m_analyticsPanel && m_service.hasModel())
        {
            runAnalytics();
        }

        if (m_investigationMode && m_bottomTabs->widget(index) == m_timelinePanel)
        {
            m_timelinePanel->refresh();
        }

        if (m_investigationMode && m_bottomTabs->widget(index) == m_crashPanel)
        {
            m_crashPanel->refresh();
        }
    });
    connect(m_timelinePanel, &TimelinePanel::navigationRequested, this, &MainWindow::applyViewerNavigation);
    connect(m_timelinePanel, &TimelinePanel::statusMessageRequested, this, &MainWindow::updateStatus);
    m_timelinePanel->setDismissedSuggestionIds(&m_dismissedSuggestionIds);
    connect(m_crashPanel, &CrashPanel::navigationRequested, this, &MainWindow::applyViewerNavigation);
    connect(m_artifactList, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem* /*current*/, QListWidgetItem* /*previous*/) {
                onArtifactSelectionChanged();
            });
    connect(m_aiPanel, &AiPanel::investigationReady, this, &MainWindow::populateTableFromInvestigation);
    connect(m_tailCheck, &QCheckBox::toggled, this, &MainWindow::toggleTail);
    connect(m_extensionList, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem* current, QListWidgetItem* /*previous*/) {
                if (current == nullptr)
                {
                    m_extensionDetails->clear();

                    return;
                }

                showSelectedExtension();
            });

    refreshSessions();
    refreshExtensions();
    updateInvestigationTabAvailability();
    switchToBottomTab(QStringLiteral("Results"));
}

void MainWindow::loadConfigurationFile()
{
    const QString path =
        QFileDialog::getOpenFileName(this, QStringLiteral("Load configuration"), QString{},
                                     QStringLiteral("Properties (*.properties)"));

    if (path.isEmpty())
    {
        return;
    }

    const auto result = m_service.loadConfiguration(scope::foundation::Path(path.toStdString()));

    if (!result || !*result)
    {
        QMessageBox::warning(this, QStringLiteral("Configuration"),
                             QStringLiteral("Could not load configuration file."));

        return;
    }

    const auto validateResult = m_service.validateConfiguration();

    if (!validateResult || !*validateResult)
    {
        const QString detail =
            !validateResult ? QString::fromStdString(validateResult.error().message())
                            : QStringLiteral("Configuration validation failed.");

        QMessageBox::warning(this, QStringLiteral("Configuration"), detail);

        return;
    }

    refreshExtensions();
    updateStatus(QStringLiteral("Loaded configuration: %1").arg(path));
    QMessageBox::information(this, QStringLiteral("Configuration"), QStringLiteral("Configuration is valid."));
}

void MainWindow::showConfigurationEditor()
{
    ConfigurationEditorDialog dialog(m_service.configurationManager(),
                                     QString::fromStdString(m_service.configFilePath().string()), this);
    dialog.exec();

    if (!dialog.configurationChanged())
    {
        return;
    }

    const scope::foundation::Path configPath =
        scope::foundation::Path(dialog.configFilePath().toStdString());

    if (!configPath.string().empty())
    {
        const auto reloadResult = m_service.loadConfiguration(configPath);

        if (!reloadResult || !*reloadResult)
        {
            QMessageBox::warning(this, QStringLiteral("Configuration"),
                                 QStringLiteral("Saved but could not reload configuration."));

            return;
        }
    }

    refreshExtensions();
    updateStatus(QStringLiteral("Configuration updated"));
}

void MainWindow::openFile()
{
    OpenLogDialog dialog(this);

    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    const QString path = dialog.logPath();

    if (path.isEmpty())
    {
        return;
    }

    if (!openLogFile(path, dialog.logFormat(), dialog.profile()))
    {
        QMessageBox::warning(this, QStringLiteral("Open failed"),
                             QStringLiteral("Could not open the selected log file."));
    }
}

void MainWindow::openFromClipboard()
{
    const QString text = QGuiApplication::clipboard()->text();

    if (text.trimmed().isEmpty())
    {
        QMessageBox::information(this, QStringLiteral("Clipboard"), QStringLiteral("Clipboard is empty."));

        return;
    }

    if (!openFromClipboardText(text))
    {
        QMessageBox::warning(this, QStringLiteral("Open failed"),
                             QStringLiteral("Could not analyze clipboard contents."));
    }
}

void MainWindow::openStdin()
{
    if (!openLogFile(QStringLiteral("-")))
    {
        QMessageBox::warning(this, QStringLiteral("Stdin"),
                             QStringLiteral("Could not read log data from stdin. Pipe input to logscope-desktop or use Open from Clipboard."));
    }
}

bool MainWindow::openLogFile(const QString& path, const scope::analysis::LogFormat formatHint,
                             const std::string& profile)
{
    if (path.isEmpty())
    {
        return false;
    }

    m_currentPath = path;
    m_formatHint = formatHint;
    m_profile = profile;
    scope::source::OpenOptions options;
    options.follow = m_tailCheck->isChecked();

    const auto openResult = m_service.openSource(scope::foundation::Path(path.toStdString()), options);

    if (!openResult)
    {
        updateStatus(QString::fromStdString(openResult.error().message()));

        return false;
    }

    updateStatus(QStringLiteral("Opened %1").arg(path));
    runAnalyze();

    return m_service.hasModel();
}

int MainWindow::logRowCount() const
{
    return m_logModel != nullptr ? m_logModel->rowCount() : 0;
}

QString MainWindow::statusMessage() const
{
    return statusBar()->currentMessage();
}

QString MainWindow::aiOutputText() const
{
    return m_aiPanel != nullptr ? m_aiPanel->outputText() : QString{};
}

bool MainWindow::runAiAsk(const QString& query)
{
    if (m_aiPanel == nullptr)
    {
        return false;
    }

    m_aiPanel->submitAsk(query);

    return !m_aiPanel->outputText().startsWith(QStringLiteral("Type a question"));
}

void MainWindow::setPersistIndexEnabled(const bool enabled)
{
    if (m_persistIndexCheck != nullptr)
    {
        m_persistIndexCheck->setChecked(enabled);
    }
}

void MainWindow::setReuseIndexEnabled(const bool enabled)
{
    if (m_reuseIndexCheck != nullptr)
    {
        m_reuseIndexCheck->setChecked(enabled);
    }
}

void MainWindow::setInvestigationLevel(const QString& level)
{
    if (m_levelEdit != nullptr)
    {
        m_levelEdit->setText(level);
    }
}

bool MainWindow::investigateCurrentFilters()
{
    if (!m_service.hasModel())
    {
        return false;
    }

    QString errorMessage;
    const scope::investigation::InvestigationCriteria criteria = buildInvestigationCriteriaFromUi(&errorMessage);

    if (!errorMessage.isEmpty())
    {
        return false;
    }

    const auto result = m_service.investigate(criteria);

    if (!result)
    {
        return false;
    }

    populateTableFromInvestigation(*result);
    updateStatus(QStringLiteral("Investigation: %1 matches").arg(static_cast<qulonglong>(result->matchingLines.size())));

    return true;
}

bool MainWindow::openFromClipboardText(const QString& text)
{
    if (text.trimmed().isEmpty())
    {
        return false;
    }

    const QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    const QString tempPath = tempDir + QStringLiteral("/logscope-clipboard.log");

    QFile file(tempPath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        return false;
    }

    file.write(text.toUtf8());
    file.close();

    return openLogFile(tempPath);
}

bool MainWindow::saveSessionToPath(const QString& path)
{
    if (!m_service.hasModel() || path.isEmpty())
    {
        return false;
    }

    SaveSessionDialog dialog(defaultReportOptions(m_service.configurationManager()), nullptr);

    QString criteriaError;
    const scope::investigation::InvestigationCriteria criteria = buildInvestigationCriteriaFromUi(&criteriaError);

    if (!criteriaError.isEmpty())
    {
        return false;
    }

    const scope::application::SessionSaveRequest request =
        dialog.sessionSaveRequest(scope::foundation::Path(path.toStdString()), m_service.configFilePath(), criteria);

    const auto saveResult = m_service.saveSession(request);

    return saveResult && *saveResult;
}

bool MainWindow::loadSessionFromPath(const QString& path)
{
    if (path.isEmpty())
    {
        return false;
    }

    const auto sessionResult = m_service.loadSession(scope::foundation::Path(path.toStdString()));

    if (!sessionResult)
    {
        return false;
    }

    m_service.adoptModel(sessionResult->analysisModel(), sessionResult->sourcePath());
    m_currentPath = QString::fromStdString(sessionResult->sourcePath().string());
    populateTableFromModel();

    if (m_logModel->rowCount() == 0 && !m_currentPath.isEmpty())
    {
        (void)openLogFile(m_currentPath);
    }

    updateStatus(QStringLiteral("Session loaded"));

    return true;
}

void MainWindow::runAnalyze()
{
    if (m_currentPath.isEmpty())
    {
        QMessageBox::information(this, QStringLiteral("Analyze"), QStringLiteral("Open a log file first."));

        return;
    }

    scope::analysis::AnalysisStats stats;
    const scope::analysis::AnalysisConfig analysisConfig =
        buildAnalysisConfigForDesktop(m_service.configurationManager(), m_persistIndexCheck->isChecked(),
                                      m_reuseIndexCheck->isChecked(), m_formatHint, m_profile);
    const auto modelResult = m_service.analyze(analysisConfig, &stats);

    if (!modelResult)
    {
        QMessageBox::warning(this, QStringLiteral("Analyze failed"), QString::fromStdString(modelResult.error().message()));

        return;
    }

    m_lastAnalysisStats = stats;
    m_hasRunStats = true;

    populateTableFromModel();

    updateStatus(QStringLiteral("Analyzed %1 lines (%2 ms)")
                       .arg(static_cast<qulonglong>(modelResult->totalLines()))
                       .arg(static_cast<double>(stats.parseDuration.totalNanoseconds()) / 1'000'000.0, 0, 'f', 3));
}

void MainWindow::populateTableFromModel()
{
    if (!m_service.hasModel())
    {
        return;
    }

    const auto& model = m_service.model();
    const std::vector<scope::analysis::IndexedLine> lines = scope::analysis::fetchIndexedLines(model);
    m_logModel->setLines(lines);

    if (lines.empty() && model.totalLines() > 0U)
    {
        updateStatus(QStringLiteral("Analyzed %1 lines but the table index is empty. Uncheck Persist/Reuse index, then Analyze again.")
                             .arg(static_cast<qulonglong>(model.totalLines())));
    }
}

void MainWindow::populateTableFromInvestigation(const scope::investigation::InvestigationResult& result)
{
    m_logModel->setLines(result.matchingLines);
}

void MainWindow::runInvestigate()
{
    if (!investigateCurrentFilters())
    {
        if (!m_service.hasModel())
        {
            QMessageBox::information(this, QStringLiteral("Investigate"), QStringLiteral("Analyze first."));
        }
        else
        {
            QString errorMessage;
            const scope::investigation::InvestigationCriteria criteria = buildInvestigationCriteriaFromUi(&errorMessage);

            if (!errorMessage.isEmpty())
            {
                QMessageBox::warning(this, QStringLiteral("Investigate"), errorMessage);
            }
            else
            {
                QMessageBox::warning(this, QStringLiteral("Investigate failed"),
                                     QStringLiteral("Investigation failed."));
            }

            Q_UNUSED(criteria);
        }
    }
}

void MainWindow::runAnalytics()
{
    if (!m_service.hasModel())
    {
        QMessageBox::information(this, QStringLiteral("Analytics"), QStringLiteral("Analyze first."));

        return;
    }

    const auto analyticsResult = m_service.runAnalytics(scope::analytics::AnalyticsConfig{});

    if (!analyticsResult)
    {
        QMessageBox::warning(this, QStringLiteral("Analytics failed"), QString::fromStdString(analyticsResult.error().message()));

        return;
    }

    m_analyticsPanel->showAnalytics(*analyticsResult);
    updateStatus(QStringLiteral("Analytics complete"));
}

void MainWindow::exportReport()
{
    if (!m_service.hasModel())
    {
        QMessageBox::information(this, QStringLiteral("Export"), QStringLiteral("Analyze first."));

        return;
    }

    ExportReportDialog dialog(defaultReportOptions(m_service.configurationManager()), this);

    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    const scope::reporting::ReportOptions options = dialog.reportOptions();
    const QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Export report"));

    if (path.isEmpty())
    {
        return;
    }

    const scope::reporting::Report report = m_service.generateReport(options);
    std::ostringstream stream;
    std::ostringstream errors;
    const auto writeResult =
        scope::cli::writeReport(report, scope::foundation::Path(path.toStdString()), stream, errors);

    if (!writeResult)
    {
        QMessageBox::warning(this, QStringLiteral("Export failed"), QString::fromStdString(writeResult.error().message()));
    }
    else
    {
        updateStatus(QStringLiteral("Exported to %1").arg(path));
    }
}

void MainWindow::showRunStats()
{
    if (!m_hasRunStats)
    {
        QMessageBox::information(this, QStringLiteral("Statistics"), QStringLiteral("Run Analyze first."));

        return;
    }

    RunStatsDialog dialog(m_lastAnalysisStats, m_service.lastPluginStats(), this);
    dialog.exec();
}

void MainWindow::saveSession()
{
    if (!m_service.hasModel())
    {
        QMessageBox::information(this, QStringLiteral("Session"), QStringLiteral("Analyze first."));

        return;
    }

    const QString path =
        QFileDialog::getSaveFileName(this, QStringLiteral("Save session"), QString{},
                                     QStringLiteral("Session (*.logscope-session)"));

    if (path.isEmpty())
    {
        return;
    }

    SaveSessionDialog dialog(defaultReportOptions(m_service.configurationManager()), this);

    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    QString criteriaError;
    const scope::investigation::InvestigationCriteria criteria = buildInvestigationCriteriaFromUi(&criteriaError);

    if (!criteriaError.isEmpty())
    {
        QMessageBox::warning(this, QStringLiteral("Session"), criteriaError);

        return;
    }

    const scope::application::SessionSaveRequest request =
        dialog.sessionSaveRequest(scope::foundation::Path(path.toStdString()), m_service.configFilePath(), criteria);

    const auto saveResult = m_service.saveSession(request);

    if (!saveResult || !*saveResult)
    {
        QMessageBox::warning(this, QStringLiteral("Save failed"), QStringLiteral("Could not save session."));

        return;
    }

    refreshSessions();
    updateStatus(QStringLiteral("Session saved"));
}

void MainWindow::loadSession()
{
    const QString path =
        QFileDialog::getOpenFileName(this, QStringLiteral("Load session"), QString{}, QStringLiteral("Session (*.logscope-session)"));

    if (path.isEmpty())
    {
        return;
    }

    const auto sessionResult = m_service.loadSession(scope::foundation::Path(path.toStdString()));

    if (!sessionResult)
    {
        QMessageBox::warning(this, QStringLiteral("Load failed"), QString::fromStdString(sessionResult.error().message()));

        return;
    }

    m_service.adoptModel(sessionResult->analysisModel(), sessionResult->sourcePath());
    m_currentPath = QString::fromStdString(sessionResult->sourcePath().string());
    populateTableFromModel();

    if (m_logModel->rowCount() == 0 && !m_currentPath.isEmpty())
    {
        if (!openLogFile(m_currentPath))
        {
            QMessageBox::warning(this, QStringLiteral("Open failed"),
                                 QStringLiteral("Session loaded, but the source log could not be reopened."));
        }
    }

    updateStatus(QStringLiteral("Session loaded"));
}

void MainWindow::refreshSessions()
{
    m_sessionList->clear();

    const auto listResult = m_service.listSessions(scope::foundation::Path("."));

    if (!listResult)
    {
        return;
    }

    for (const scope::foundation::Path& sessionPath : *listResult)
    {
        m_sessionList->addItem(QString::fromStdString(sessionPath.string()));
    }
}

void MainWindow::refreshExtensions()
{
    m_extensionList->clear();
    m_extensionDetails->clear();

    for (const scope::extension::ExtensionInfo& info : m_service.listExtensions())
    {
        m_extensionList->addItem(QString::fromStdString(info.id));
    }

    if (m_extensionList->count() > 0)
    {
        m_extensionList->setCurrentRow(0);
    }
}

void MainWindow::showSelectedExtension()
{
    QListWidgetItem* item = m_extensionList->currentItem();

    if (item == nullptr)
    {
        m_extensionDetails->clear();

        return;
    }

    const auto infoResult = m_service.describeExtension(item->text().toStdString());

    if (!infoResult)
    {
        m_extensionDetails->setPlainText(QString::fromStdString(infoResult.error().message()));

        return;
    }

    const scope::extension::ExtensionInfo& info = *infoResult;
    QString details = QStringLiteral("ID: %1\nVersion: %2\nEnabled: %3\nStatus: %4\n")
                          .arg(QString::fromStdString(info.id), QString::fromStdString(info.version),
                               info.enabled ? QStringLiteral("yes") : QStringLiteral("no"),
                               extensionStatusLabel(info.status));

    if (info.dynamic)
    {
        details += QStringLiteral("Source: dynamic\nAPI version: %1\n").arg(info.apiVersion);

        if (!info.libraryPath.empty())
        {
            details += QStringLiteral("Library: %1\n").arg(QString::fromStdString(info.libraryPath));
        }
    }

    details += QStringLiteral("Description: %1").arg(QString::fromStdString(info.description));
    m_extensionDetails->setPlainText(details);
}

void MainWindow::toggleTail(const bool enabled)
{
    if (!enabled)
    {
        m_tailWorker->stop();
        m_service.stopTail();

        return;
    }

    if (m_currentPath.isEmpty())
    {
        m_tailCheck->setChecked(false);

        return;
    }

    const auto startResult = m_service.startTail();

    if (!startResult || !*startResult)
    {
        QMessageBox::warning(this, QStringLiteral("Tail failed"), QStringLiteral("Could not start tail."));

        m_tailCheck->setChecked(false);

        return;
    }

    m_tailWorker->start();
    updateStatus(QStringLiteral("Tail active"));
}

void MainWindow::applyLightTheme()
{
    ThemeManager::apply(ThemeMode::Light);
}

void MainWindow::applyDarkTheme()
{
    ThemeManager::apply(ThemeMode::Dark);
}

void MainWindow::updateStatus(const QString& message)
{
    statusBar()->showMessage(message);
}

scope::investigation::InvestigationCriteria MainWindow::buildInvestigationCriteriaFromUi(QString* errorMessage) const
{
    scope::investigation::InvestigationCriteria criteria;
    criteria.contentSearch = m_searchEdit->text().toStdString();
    criteria.booleanQuery = m_queryEdit->text().toStdString();
    criteria.filterExpression = m_filterEdit->text().toStdString();

    if (!m_levelEdit->text().isEmpty())
    {
        const std::string level = m_levelEdit->text().toStdString();

        if (level == "error")
        {
            criteria.field =
                scope::investigation::FieldFilter::any().withLevel(scope::analysis::DetectedLogLevel::Error);
        }
        else if (level == "warning" || level == "warn")
        {
            criteria.field =
                scope::investigation::FieldFilter::any().withLevel(scope::analysis::DetectedLogLevel::Warn);
        }
        else if (level == "info")
        {
            criteria.field =
                scope::investigation::FieldFilter::any().withLevel(scope::analysis::DetectedLogLevel::Info);
        }
    }

    if (!m_timeFromEdit->text().isEmpty())
    {
        const auto earliest = scope::foundation::Timestamp::parse(m_timeFromEdit->text().toStdString());

        if (!earliest)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Invalid time-from timestamp (use ISO-like format).");
            }

            return criteria;
        }

        criteria.timeRange = criteria.timeRange.withEarliest(*earliest);
    }

    if (!m_timeToEdit->text().isEmpty())
    {
        const auto latest = scope::foundation::Timestamp::parse(m_timeToEdit->text().toStdString());

        if (!latest)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Invalid time-to timestamp (use ISO-like format).");
            }

            return criteria;
        }

        criteria.timeRange = criteria.timeRange.withLatest(*latest);
    }

    if (m_regexCheck->isChecked())
    {
        criteria.searchMode = scope::search::SearchMode::Regex;
    }

    if (m_caseCheck->isChecked())
    {
        criteria.caseSensitivity = scope::search::CaseSensitivity::Sensitive;
    }

    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }

    return criteria;
}

void MainWindow::promptCreateInvestigation()
{
    bool ok = false;
    const QString name =
        QInputDialog::getText(this, QStringLiteral("New Investigation"), QStringLiteral("Name:"),
                              QLineEdit::Normal, QStringLiteral("desktop-investigation"), &ok);

    if (!ok || name.trimmed().isEmpty())
    {
        return;
    }

    if (!createInvestigation(name.trimmed()))
    {
        QMessageBox::warning(this, QStringLiteral("Investigation"),
                             QStringLiteral("Could not create investigation."));
    }
}

void MainWindow::openInvestigation()
{
    const QString path =
        QFileDialog::getExistingDirectory(this, QStringLiteral("Open Investigation"), QString{});

    if (path.isEmpty())
    {
        return;
    }

    const auto openResult = m_investigationController.open(scope::foundation::Path(path.toStdString()));

    if (!openResult)
    {
        QMessageBox::warning(this, QStringLiteral("Investigation"),
                             QString::fromStdString(openResult.error().message()));

        return;
    }

    setInvestigationMode(true);
    updateStatus(QStringLiteral("Investigation: %1").arg(QString::fromStdString(openResult->name)));
}

void MainWindow::closeInvestigation()
{
    m_dismissedSuggestionIds.clear();
    m_investigationController.close();
    setInvestigationMode(false);
    updateStatus(QStringLiteral("Investigation closed"));
}

void MainWindow::addInvestigationArtifact()
{
    if (!m_investigationMode)
    {
        QMessageBox::information(this, QStringLiteral("Investigation"),
                                 QStringLiteral("Create or open an investigation first."));

        return;
    }

    const QString path =
        QFileDialog::getOpenFileName(this, QStringLiteral("Add Artifact"), QString{},
                                     QStringLiteral("All files (*.*)"));

    if (path.isEmpty())
    {
        return;
    }

    const scope::foundation::Path sourcePath(path.toStdString());
    const std::string type = scope::application::InvestigationController::inferArtifactType({}, sourcePath);

    const auto result = type == "log" ? m_investigationController.addLogArtifact(sourcePath)
                                      : m_investigationController.addArtifactFile(sourcePath, type);

    if (!result)
    {
        QMessageBox::warning(this, QStringLiteral("Investigation"),
                             QString::fromStdString(result.error().message()));

        return;
    }

    refreshArtifactList();
    updateInvestigationTabAvailability();
    m_timelinePanel->refresh();

    if (type == "pstack" || type == "core")
    {
        m_crashPanel->setActiveArtifactId(QString::fromStdString(result->id));
        m_crashPanel->refresh();
    }

    updateStatus(QStringLiteral("Added artifact: %1").arg(path));
}

void MainWindow::refreshArtifactList()
{
    m_artifactList->blockSignals(true);
    m_artifactList->clear();

    if (!m_investigationController.isOpen())
    {
        return;
    }

    for (const scope::workspace::ArtifactRecord& artifact : m_investigationController.manifest().artifacts)
    {
        auto* item = new QListWidgetItem(
            QStringLiteral("%1 (%2)").arg(QString::fromStdString(artifact.name), QString::fromStdString(artifact.type)));
        item->setData(Qt::UserRole, QString::fromStdString(artifact.id));
        item->setData(Qt::UserRole + 1, QString::fromStdString(artifact.type));
        m_artifactList->addItem(item);
    }

    m_artifactList->blockSignals(false);
}

void MainWindow::setInvestigationMode(const bool enabled)
{
    m_investigationMode = enabled;
    m_sessionList->setVisible(!enabled);
    m_artifactList->setVisible(enabled);
    refreshArtifactList();
    m_timelinePanel->setInvestigationActive(enabled);
    m_crashPanel->setInvestigationActive(enabled);
    updateInvestigationTabAvailability();

    if (enabled)
    {
        setWindowTitle(QStringLiteral("LogScope Desktop — %1")
                           .arg(QString::fromStdString(m_investigationController.manifest().name)));
    }
    else
    {
        setWindowTitle(QStringLiteral("LogScope Desktop"));
    }
}

void MainWindow::updateInvestigationTabAvailability()
{
    const bool enabled = m_investigationMode && m_investigationController.isOpen();

    if (!enabled && (m_bottomTabs->currentWidget() == m_timelinePanel
                     || m_bottomTabs->currentWidget() == m_crashPanel))
    {
        switchToBottomTab(QStringLiteral("Results"));
    }

    m_bottomTabs->setTabEnabled(m_bottomTabs->indexOf(m_timelinePanel), enabled);

    bool hasCrashArtifact = false;

    if (enabled)
    {
        for (const scope::workspace::ArtifactRecord& artifact : m_investigationController.manifest().artifacts)
        {
            if (artifact.type == "pstack" || artifact.type == "core")
            {
                hasCrashArtifact = true;
                break;
            }
        }
    }

    m_bottomTabs->setTabEnabled(m_bottomTabs->indexOf(m_crashPanel), enabled && hasCrashArtifact);
}

void MainWindow::switchToBottomTab(const QString& tabName)
{
    for (int index = 0; index < m_bottomTabs->count(); ++index)
    {
        if (m_bottomTabs->tabText(index).compare(tabName, Qt::CaseInsensitive) == 0)
        {
            m_bottomTabs->setCurrentIndex(index);

            return;
        }
    }
}

void MainWindow::applyViewerNavigation(const ViewerNavigation& navigation)
{
    if (navigation.targetTab == QStringLiteral("crash"))
    {
        if (!navigation.artifactId.isEmpty())
        {
            m_crashPanel->setActiveArtifactId(navigation.artifactId);
            m_crashPanel->refresh();
        }

        switchToBottomTab(QStringLiteral("Crash"));

        if (!navigation.statusMessage.isEmpty())
        {
            updateStatus(navigation.statusMessage);
        }

        return;
    }

    if (navigation.targetTab == QStringLiteral("results") && !navigation.artifactId.isEmpty())
    {
        openInvestigationLogArtifact(navigation.artifactId, navigation.lineNumber);
        switchToBottomTab(QStringLiteral("Results"));

        if (!navigation.statusMessage.isEmpty())
        {
            updateStatus(navigation.statusMessage);
        }

        return;
    }

    if (!navigation.targetTab.isEmpty())
    {
        switchToBottomTab(navigation.targetTab);
    }

    if (!navigation.statusMessage.isEmpty())
    {
        updateStatus(navigation.statusMessage);
    }
}

void MainWindow::openInvestigationLogArtifact(const QString& artifactId,
                                              const std::optional<std::size_t> highlightLine)
{
    const auto pathResult = m_investigationController.resolveLogArtifactPath(artifactId.toStdString());

    if (!pathResult)
    {
        updateStatus(QString::fromStdString(pathResult.error().message()));

        return;
    }

    const QString path = QString::fromStdString(pathResult->string());

    if (!openLogFile(path))
    {
        return;
    }

    runAnalyze();

    if (highlightLine.has_value())
    {
        for (int row = 0; row < m_logModel->rowCount(); ++row)
        {
            const QModelIndex index = m_logModel->index(row, 0);
            const QVariant lineValue = m_logModel->data(index, Qt::DisplayRole);

            if (lineValue.toULongLong() == *highlightLine)
            {
                m_logView->selectRow(row);
                m_logView->scrollTo(index, QAbstractItemView::PositionAtCenter);

                break;
            }
        }
    }
}

void MainWindow::onArtifactSelectionChanged()
{
    QListWidgetItem* item = m_artifactList->currentItem();

    if (item == nullptr || !m_investigationMode)
    {
        return;
    }

    const QString artifactId = item->data(Qt::UserRole).toString();
    const QString artifactType = item->data(Qt::UserRole + 1).toString();

    if (artifactType == QStringLiteral("log"))
    {
        openInvestigationLogArtifact(artifactId, std::nullopt);
    }
    else if (artifactType == QStringLiteral("pstack") || artifactType == QStringLiteral("core"))
    {
        m_crashPanel->setActiveArtifactId(artifactId);
        m_crashPanel->refresh();
        switchToBottomTab(QStringLiteral("Crash"));
    }
}

bool MainWindow::createInvestigation(const QString& name)
{
    const auto createResult = m_investigationController.create(name.toStdString());

    if (!createResult)
    {
        return false;
    }

    setInvestigationMode(true);
    updateStatus(QStringLiteral("Investigation: %1").arg(name));

    return true;
}

bool MainWindow::addInvestigationLogArtifact(const QString& path)
{
    if (!m_investigationMode)
    {
        return false;
    }

    const auto result = m_investigationController.addLogArtifact(scope::foundation::Path(path.toStdString()));

    if (!result)
    {
        return false;
    }

    refreshArtifactList();
    updateInvestigationTabAvailability();

    if (m_timelinePanel != nullptr)
    {
        m_timelinePanel->refresh();
    }

    return true;
}

bool MainWindow::addInvestigationPstackArtifact(const QString& path)
{
    if (!m_investigationMode)
    {
        return false;
    }

    const auto result =
        m_investigationController.addArtifactFile(scope::foundation::Path(path.toStdString()), "pstack");

    if (!result)
    {
        return false;
    }

    refreshArtifactList();
    updateInvestigationTabAvailability();
    m_crashPanel->setActiveArtifactId(QString::fromStdString(result->id));

    if (m_timelinePanel != nullptr)
    {
        m_timelinePanel->refresh();
    }

    return true;
}

bool MainWindow::switchBottomTab(const QString& tabName)
{
    switchToBottomTab(tabName);

    return m_bottomTabs->tabText(m_bottomTabs->currentIndex()).compare(tabName, Qt::CaseInsensitive) == 0;
}

bool MainWindow::selectTimelineCrashSummary()
{
    switchToBottomTab(QStringLiteral("Timeline"));
    m_timelinePanel->refresh();

    if (!m_timelinePanel->waitForLoadComplete())
    {
        return false;
    }

    if (!m_timelinePanel->selectRowByEventType(QStringLiteral("crash.summary")))
    {
        return false;
    }

    return waitForCrashLoad();
}

bool MainWindow::clickCrashFaultThread()
{
    switchToBottomTab(QStringLiteral("Crash"));

    if (!waitForCrashLoad())
    {
        return false;
    }

    return m_crashPanel->clickFaultThread();
}

QString MainWindow::crashSignalText() const
{
    return m_crashPanel->signalText();
}

bool MainWindow::investigationModeActive() const
{
    return m_investigationMode;
}

bool MainWindow::closeActiveInvestigation()
{
    if (!m_investigationMode)
    {
        return false;
    }

    closeInvestigation();

    return !m_investigationMode;
}

bool MainWindow::openInvestigationAtPath(const QString& path)
{
    const auto openResult =
        m_investigationController.open(scope::foundation::Path(path.toStdString()));

    if (!openResult)
    {
        return false;
    }

    setInvestigationMode(true);
    updateStatus(QStringLiteral("Investigation: %1").arg(QString::fromStdString(openResult->name)));

    if (m_timelinePanel != nullptr)
    {
        m_timelinePanel->refresh();
    }

    return true;
}

bool MainWindow::isBottomTabEnabled(const QString& tabName) const
{
    for (int index = 0; index < m_bottomTabs->count(); ++index)
    {
        if (m_bottomTabs->tabText(index).compare(tabName, Qt::CaseInsensitive) == 0)
        {
            return m_bottomTabs->isTabEnabled(index);
        }
    }

    return false;
}

int MainWindow::investigationArtifactCount() const
{
    if (!m_investigationController.isOpen())
    {
        return 0;
    }

    return static_cast<int>(m_investigationController.manifest().artifacts.size());
}

QString MainWindow::investigationDirectoryPath() const
{
    if (!m_investigationController.isOpen())
    {
        return {};
    }

    return QString::fromStdString(m_investigationController.investigationDirectory().string());
}

bool MainWindow::timelineHasEventType(const QString& eventType) const
{
    return m_timelinePanel != nullptr && m_timelinePanel->hasEventType(eventType);
}

int MainWindow::timelineRowCount() const
{
    return m_timelinePanel != nullptr ? m_timelinePanel->rowCount() : 0;
}

QString MainWindow::currentBottomTabName() const
{
    if (m_bottomTabs == nullptr || m_bottomTabs->currentIndex() < 0)
    {
        return {};
    }

    return m_bottomTabs->tabText(m_bottomTabs->currentIndex());
}

bool MainWindow::waitForTimelineLoad(int timeoutMs)
{
    return m_timelinePanel != nullptr && m_timelinePanel->waitForLoadComplete(timeoutMs);
}

bool MainWindow::waitForCrashLoad(int timeoutMs)
{
    return m_crashPanel != nullptr && m_crashPanel->waitForLoadComplete(timeoutMs);
}

bool MainWindow::selectTimelineRow(int row)
{
    return m_timelinePanel != nullptr && m_timelinePanel->selectRow(row);
}

bool MainWindow::createEvidenceLinkBetweenRows(int sourceRow, int targetRow)
{
    return m_timelinePanel != nullptr && m_timelinePanel->createLinkBetweenRows(sourceRow, targetRow);
}

int MainWindow::timelineLinkBadgeCount() const
{
    return m_timelinePanel != nullptr ? m_timelinePanel->linkBadgeCount() : 0;
}

int MainWindow::relatedEvidenceRowCount() const
{
    return m_timelinePanel != nullptr ? m_timelinePanel->relatedEvidenceRowCount() : 0;
}

bool MainWindow::suggestionPanelVisible() const
{
    return m_timelinePanel != nullptr && m_timelinePanel->suggestionPanelVisible();
}

bool MainWindow::acceptFirstSuggestion()
{
    return m_timelinePanel != nullptr && m_timelinePanel->acceptFirstSuggestion();
}

bool MainWindow::dismissFirstSuggestion()
{
    return m_timelinePanel != nullptr && m_timelinePanel->dismissFirstSuggestion();
}

bool MainWindow::removeFirstEvidenceLink()
{
    return m_timelinePanel != nullptr && m_timelinePanel->removeFirstEvidenceLink();
}

bool MainWindow::openFirstRelatedEvidence()
{
    return m_timelinePanel != nullptr && m_timelinePanel->openFirstRelatedEvidence();
}

} // namespace scope::desktop
