/**
 * @file main_window.cpp
 */

#include "main_window.hpp"

#include <QLabel>
#include <QHeaderView>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplitter>
#include <QVBoxLayout>
#include <sstream>

#include "analysis_config.hpp"
#include "analytics_config.hpp"
#include "field_filter.hpp"
#include "foundation/path.hpp"
#include "foundation/timestamp.hpp"
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

} // namespace

MainWindow::MainWindow(const scope::foundation::Path& configFile, QWidget* parent) : QMainWindow(parent)
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
    fileMenu->addAction(QStringLiteral("Load Configuration…"), this, &MainWindow::loadConfigurationFile);
    fileMenu->addAction(QStringLiteral("Export Report…"), this, &MainWindow::exportReport);
    fileMenu->addSeparator();
    fileMenu->addAction(QStringLiteral("Quit"), this, &QWidget::close);

    auto* sessionMenu = menuBar()->addMenu(QStringLiteral("Session"));
    sessionMenu->addAction(QStringLiteral("Save…"), this, &MainWindow::saveSession);
    sessionMenu->addAction(QStringLiteral("Load…"), this, &MainWindow::loadSession);

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
    m_extensionList = new QListWidget(navigator);
    m_extensionDetails = new QTextEdit(navigator);
    m_extensionDetails->setReadOnly(true);
    m_extensionDetails->setPlaceholderText(QStringLiteral("Select an extension"));
    m_extensionDetails->setMaximumHeight(120);
    navLayout->addWidget(new QLabel(QStringLiteral("Sessions"), navigator));
    navLayout->addWidget(m_sessionList);
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
    auto* analyticsButton = new QPushButton(QStringLiteral("Analytics"), workArea);

    toolbar->addWidget(openButton);
    toolbar->addWidget(m_tailCheck);
    toolbar->addWidget(m_persistIndexCheck);
    toolbar->addWidget(m_reuseIndexCheck);
    toolbar->addWidget(analyzeButton);
    toolbar->addWidget(investigateButton);
    toolbar->addWidget(analyticsButton);

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

    auto* bottomSplitter = new QSplitter(Qt::Horizontal, workArea);
    m_analyticsPanel = new AnalyticsPanel(bottomSplitter);
    m_aiPanel = new AiPanel(&m_service, bottomSplitter);
    bottomSplitter->addWidget(m_analyticsPanel);
    bottomSplitter->addWidget(m_aiPanel);

    workLayout->addLayout(toolbar);
    workLayout->addLayout(filterRow);
    workLayout->addWidget(m_logView, 3);
    workLayout->addWidget(bottomSplitter, 1);

    splitter->addWidget(navigator);
    splitter->addWidget(workArea);
    splitter->setStretchFactor(1, 1);

    connect(openButton, &QPushButton::clicked, this, &MainWindow::openFile);
    connect(analyzeButton, &QPushButton::clicked, this, &MainWindow::runAnalyze);
    connect(investigateButton, &QPushButton::clicked, this, &MainWindow::runInvestigate);
    connect(analyticsButton, &QPushButton::clicked, this, &MainWindow::runAnalytics);
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

void MainWindow::openFile()
{
    const QString path =
        QFileDialog::getOpenFileName(this, QStringLiteral("Open log file"), QString{}, QStringLiteral("Logs (*.log *.jsonl *.*)"));

    if (path.isEmpty())
    {
        return;
    }

    m_currentPath = path;
    scope::source::OpenOptions options;
    options.follow = m_tailCheck->isChecked();

    const auto openResult = m_service.openSource(scope::foundation::Path(path.toStdString()), options);

    if (!openResult)
    {
        QMessageBox::warning(this, QStringLiteral("Open failed"), QString::fromStdString(openResult.error().message()));

        return;
    }

    updateStatus(QStringLiteral("Opened %1").arg(path));
    runAnalyze();
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
                                      m_reuseIndexCheck->isChecked());
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

    if (model.lineIndex().has_value())
    {
        m_logModel->setLines(model.lineIndex()->lines());
    }
    else
    {
        m_logModel->setLines({});
    }
}

void MainWindow::populateTableFromInvestigation(const scope::investigation::InvestigationResult& result)
{
    m_logModel->setLines(result.matchingLines);
}

void MainWindow::runInvestigate()
{
    if (!m_service.hasModel())
    {
        QMessageBox::information(this, QStringLiteral("Investigate"), QStringLiteral("Analyze first."));

        return;
    }

    scope::investigation::InvestigationCriteria criteria;
    criteria.contentSearch = m_searchEdit->text().toStdString();
    criteria.booleanQuery = m_queryEdit->text().toStdString();
    criteria.filterExpression = m_filterEdit->text().toStdString();

    if (!m_levelEdit->text().isEmpty())
    {
        const std::string level = m_levelEdit->text().toStdString();

        if (level == "error")
        {
            criteria.field = scope::investigation::FieldFilter::any().withLevel(scope::analysis::DetectedLogLevel::Error);
        }
        else if (level == "warning" || level == "warn")
        {
            criteria.field = scope::investigation::FieldFilter::any().withLevel(scope::analysis::DetectedLogLevel::Warn);
        }
        else if (level == "info")
        {
            criteria.field = scope::investigation::FieldFilter::any().withLevel(scope::analysis::DetectedLogLevel::Info);
        }
    }

    if (!m_timeFromEdit->text().isEmpty())
    {
        const auto earliest = scope::foundation::Timestamp::parse(m_timeFromEdit->text().toStdString());

        if (!earliest)
        {
            QMessageBox::warning(this, QStringLiteral("Investigate"),
                                 QStringLiteral("Invalid time-from timestamp (use ISO-like format)."));

            return;
        }

        criteria.timeRange = criteria.timeRange.withEarliest(*earliest);
    }

    if (!m_timeToEdit->text().isEmpty())
    {
        const auto latest = scope::foundation::Timestamp::parse(m_timeToEdit->text().toStdString());

        if (!latest)
        {
            QMessageBox::warning(this, QStringLiteral("Investigate"),
                                 QStringLiteral("Invalid time-to timestamp (use ISO-like format)."));

            return;
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

    const auto result = m_service.investigate(criteria);

    if (!result)
    {
        QMessageBox::warning(this, QStringLiteral("Investigate failed"), QString::fromStdString(result.error().message()));

        return;
    }

    populateTableFromInvestigation(*result);
    updateStatus(QStringLiteral("Investigation: %1 matches").arg(static_cast<qulonglong>(result->matchingLines.size())));
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
        QFileDialog::getSaveFileName(this, QStringLiteral("Save session"), QString{}, QStringLiteral("Session (*.logscope-session)"));

    if (path.isEmpty())
    {
        return;
    }

    scope::application::SessionSaveRequest request;
    request.sessionFile = scope::foundation::Path(path.toStdString());
    request.configFile = m_service.configFilePath();
    request.reportOptions = defaultReportOptions(m_service.configurationManager());

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

} // namespace scope::desktop
