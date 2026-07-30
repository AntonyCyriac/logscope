/**
 * @file main_window.cpp
 */

#include "main_window.hpp"

#include <QLabel>
#include <QHeaderView>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplitter>
#include <QVBoxLayout>
#include <sstream>

#include "analysis_config.hpp"
#include "analytics_config.hpp"
#include "field_filter.hpp"
#include "foundation/path.hpp"
#include "report_options.hpp"
#include "report_writer.hpp"
#include "theme_manager.hpp"
#include "time_range_filter.hpp"

namespace scope::desktop
{

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
    navLayout->addWidget(new QLabel(QStringLiteral("Sessions"), navigator));
    navLayout->addWidget(m_sessionList);
    navLayout->addWidget(new QLabel(QStringLiteral("Extensions"), navigator));
    navLayout->addWidget(m_extensionList);

    auto* workArea = new QWidget(splitter);
    auto* workLayout = new QVBoxLayout(workArea);

    auto* toolbar = new QHBoxLayout();
    auto* openButton = new QPushButton(QStringLiteral("Open"), workArea);
    m_tailCheck = new QCheckBox(QStringLiteral("Tail"), workArea);
    auto* analyzeButton = new QPushButton(QStringLiteral("Analyze"), workArea);
    auto* investigateButton = new QPushButton(QStringLiteral("Investigate"), workArea);
    auto* analyticsButton = new QPushButton(QStringLiteral("Analytics"), workArea);

    toolbar->addWidget(openButton);
    toolbar->addWidget(m_tailCheck);
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

    m_formatCombo = new QComboBox(workArea);
    m_formatCombo->addItems({QStringLiteral("text"), QStringLiteral("html"), QStringLiteral("pdf"),
                             QStringLiteral("json"), QStringLiteral("markdown"), QStringLiteral("csv")});
    toolbar->addWidget(m_formatCombo);

    connect(openButton, &QPushButton::clicked, this, &MainWindow::openFile);
    connect(analyzeButton, &QPushButton::clicked, this, &MainWindow::runAnalyze);
    connect(investigateButton, &QPushButton::clicked, this, &MainWindow::runInvestigate);
    connect(analyticsButton, &QPushButton::clicked, this, &MainWindow::runAnalytics);
    connect(m_tailCheck, &QCheckBox::toggled, this, &MainWindow::toggleTail);

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

    refreshExtensions();
    updateStatus(QStringLiteral("Loaded configuration: %1").arg(path));
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
    const auto modelResult =
        m_service.analyze(scope::analysis::resolveAnalysisConfig(m_service.configurationManager().configuration(),
                                                               scope::analysis::AnalysisConfig::defaults()),
                          &stats);

    if (!modelResult)
    {
        QMessageBox::warning(this, QStringLiteral("Analyze failed"), QString::fromStdString(modelResult.error().message()));

        return;
    }

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

    if (!m_timeFromEdit->text().isEmpty() || !m_timeToEdit->text().isEmpty())
    {
        // Time range strings are applied via filter DSL or future timestamp parsing.
        (void)m_timeFromEdit;
        (void)m_timeToEdit;
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

    const QString format = m_formatCombo->currentText();
    const QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Export report"));

    if (path.isEmpty())
    {
        return;
    }

    scope::reporting::ReportOptions options;
    options.format = scope::reporting::ReportFormat::Text;

    if (format == "html")
    {
        options.format = scope::reporting::ReportFormat::Html;
    }
    else if (format == "pdf")
    {
        options.format = scope::reporting::ReportFormat::Pdf;
    }
    else if (format == "json")
    {
        options.format = scope::reporting::ReportFormat::Json;
    }
    else if (format == "markdown")
    {
        options.format = scope::reporting::ReportFormat::Markdown;
    }
    else if (format == "csv")
    {
        options.format = scope::reporting::ReportFormat::Csv;
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
    request.reportOptions = scope::reporting::ReportOptions{};

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

    for (const scope::extension::ExtensionInfo& info : m_service.listExtensions())
    {
        m_extensionList->addItem(QString::fromStdString(info.id));
    }
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
