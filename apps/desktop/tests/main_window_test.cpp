/**
 * @file main_window_test.cpp
 * @brief Desktop GUI integration tests (headless Qt Test).
 */

#include <QtTest>

#include <QComboBox>
#include <QLineEdit>

#include "configuration_editor_dialog.hpp"
#include "configuration_manager.hpp"
#include "desktop_analysis_config.hpp"
#include "foundation/path.hpp"
#include "investigation_criteria.hpp"
#include "main_window.hpp"
#include "open_log_dialog.hpp"
#include "save_session_dialog.hpp"

namespace
{

QString sampleLogPath()
{
    return QStringLiteral("samples/sample.log");
}

QString largeAppLogPath()
{
    return QStringLiteral("samples/large-app.log");
}

QString noopConfigPath()
{
    return QStringLiteral("samples/ai-noop.properties");
}

QString uniqueTestPath(const char* suffix)
{
    return QStringLiteral("build/desktop_test_%1").arg(QString::fromUtf8(suffix));
}

QString sampleClipboardLog()
{
    return QStringLiteral(
        "2026-07-11 10:00:01 INFO Application started\n"
        "2026-07-11 10:00:05 INFO Connecting to database\n"
        "2026-07-11 10:00:06 ERROR Connection refused\n"
        "2026-07-11 10:00:07 ERROR Connection refused\n"
        "2026-07-11 10:00:10 WARNING Request taking too long\n"
        "2026-07-11 10:00:15 ERROR Request timeout\n"
        "2026-07-11 10:00:20 INFO Retrying request\n"
        "2026-07-11 10:00:25 ERROR Connection refused\n");
}

} // namespace

class MainWindowTest : public QObject
{
    Q_OBJECT

  private slots:
    void openAndAnalyzeShowsEightLines();
    void askErrorsShowsFourMatches();
    void persistIndexStillShowsEightLines();
    void openWithGenericPlainProfileShowsEightLines();
    void largeAppLogPersistIndexAnalyzesAllLines();
    void openFromClipboardShowsEightLines();
    void investigateLevelErrorShowsFourRows();
    void saveAndLoadSessionRoundTrip();
    void openLogDialogFormatAndProfile();
    void saveSessionDialogIncludesContentCriteria();
    void configurationEditorValidatesLoadedConfig();
    void configurationEditorRejectsInvalidProfile();
};

void MainWindowTest::openAndAnalyzeShowsEightLines()
{
    scope::desktop::MainWindow window(scope::foundation::Path(noopConfigPath().toStdString()));
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QVERIFY(window.openLogFile(sampleLogPath()));
    QCOMPARE(window.logRowCount(), 8);
    QVERIFY(window.statusMessage().contains(QStringLiteral("Analyzed")));
}

void MainWindowTest::askErrorsShowsFourMatches()
{
    scope::desktop::MainWindow window(scope::foundation::Path(noopConfigPath().toStdString()));
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QVERIFY(window.openLogFile(sampleLogPath()));
    QCOMPARE(window.logRowCount(), 8);

    QVERIFY(window.runAiAsk(QStringLiteral("errors")));
    QCOMPARE(window.logRowCount(), 4);
    QVERIFY(window.aiOutputText().contains(QStringLiteral("Matches: 4")));
}

void MainWindowTest::persistIndexStillShowsEightLines()
{
    scope::desktop::MainWindow window(scope::foundation::Path(noopConfigPath().toStdString()));
    window.setPersistIndexEnabled(true);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QVERIFY(window.openLogFile(sampleLogPath()));
    QCOMPARE(window.logRowCount(), 8);
}

void MainWindowTest::openWithGenericPlainProfileShowsEightLines()
{
    scope::desktop::MainWindow window(scope::foundation::Path(noopConfigPath().toStdString()));
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QVERIFY(window.openLogFile(sampleLogPath(), scope::analysis::LogFormat::PlainText, "generic-plain"));
    QCOMPARE(window.logRowCount(), 8);
}

void MainWindowTest::largeAppLogPersistIndexAnalyzesAllLines()
{
    scope::desktop::MainWindow window(scope::foundation::Path(noopConfigPath().toStdString()));
    window.setPersistIndexEnabled(true);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QVERIFY(window.openLogFile(largeAppLogPath()));
    QVERIFY(window.statusMessage().contains(QStringLiteral("13913")));
    QCOMPARE(window.logRowCount(), 13913);
}

void MainWindowTest::openFromClipboardShowsEightLines()
{
    scope::desktop::MainWindow window(scope::foundation::Path(noopConfigPath().toStdString()));
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QVERIFY(window.openFromClipboardText(sampleClipboardLog()));
    QCOMPARE(window.logRowCount(), 8);
}

void MainWindowTest::investigateLevelErrorShowsFourRows()
{
    scope::desktop::MainWindow window(scope::foundation::Path(noopConfigPath().toStdString()));
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QVERIFY(window.openLogFile(sampleLogPath()));
    QCOMPARE(window.logRowCount(), 8);

    window.setInvestigationLevel(QStringLiteral("error"));
    QVERIFY(window.investigateCurrentFilters());
    QCOMPARE(window.logRowCount(), 4);
    QVERIFY(window.statusMessage().contains(QStringLiteral("4 matches")));
}

void MainWindowTest::saveAndLoadSessionRoundTrip()
{
    scope::desktop::MainWindow window(scope::foundation::Path(noopConfigPath().toStdString()));
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QVERIFY(window.openLogFile(sampleLogPath()));
    QCOMPARE(window.logRowCount(), 8);

    const QString sessionPath = uniqueTestPath("session_roundtrip.logscope-session");
    QVERIFY(window.saveSessionToPath(sessionPath));

    scope::desktop::MainWindow reloadWindow(scope::foundation::Path(noopConfigPath().toStdString()));
    reloadWindow.show();
    QVERIFY(QTest::qWaitForWindowExposed(&reloadWindow));

    QVERIFY(reloadWindow.loadSessionFromPath(sessionPath));
    QCOMPARE(reloadWindow.logRowCount(), 8);
}

void MainWindowTest::openLogDialogFormatAndProfile()
{
    scope::desktop::OpenLogDialog dialog;
    auto* pathEdit = dialog.findChild<QLineEdit*>();

    QVERIFY(pathEdit != nullptr);
    pathEdit->setText(sampleLogPath());

    auto* formatCombo = dialog.findChild<QComboBox*>();

    QVERIFY(formatCombo != nullptr);
    formatCombo->setCurrentText(QStringLiteral("plain"));

    const QList<QComboBox*> combos = dialog.findChildren<QComboBox*>();

    QVERIFY(combos.size() >= 2);
    combos.at(1)->setCurrentIndex(1); // generic-plain

    QCOMPARE(dialog.logPath(), sampleLogPath());
    QCOMPARE(dialog.logFormat(), scope::analysis::LogFormat::PlainText);
    QCOMPARE(dialog.profile(), "generic-plain");
}

void MainWindowTest::saveSessionDialogIncludesContentCriteria()
{
    scope::configuration::ConfigurationManager configurationManager;
    const scope::reporting::ReportOptions defaults =
        scope::desktop::defaultReportOptions(configurationManager);

    scope::desktop::SaveSessionDialog dialog(defaults);

    scope::investigation::InvestigationCriteria criteria;
    criteria.contentSearch = "timeout";

    const scope::application::SessionSaveRequest request =
        dialog.sessionSaveRequest(scope::foundation::Path("session.logscope-session"),
                                  scope::foundation::Path("config.properties"), criteria);

    QCOMPARE(request.contentCriteria.contentSearch, std::string("timeout"));
    QCOMPARE(request.levelFilter.minimumErrors(), 0U);
}

void MainWindowTest::configurationEditorValidatesLoadedConfig()
{
    const auto loadResult =
        scope::configuration::ConfigurationManager::loadFromFile(scope::foundation::Path(noopConfigPath().toStdString()));

    QVERIFY(loadResult.hasValue());

    scope::configuration::ConfigurationManager manager = std::move(loadResult.value());
    scope::desktop::ConfigurationEditorDialog dialog(manager, noopConfigPath());
    QVERIFY(dialog.validateCurrent().isEmpty());
}

void MainWindowTest::configurationEditorRejectsInvalidProfile()
{
    scope::configuration::ConfigurationManager manager;
    manager.configuration().set("profile", "not-a-real-profile");

    scope::desktop::ConfigurationEditorDialog dialog(manager, QString{});
    const QString error = dialog.validateCurrent();

    QVERIFY(!error.isEmpty());
    QVERIFY(error.contains(QStringLiteral("profile"), Qt::CaseInsensitive));
}

QTEST_MAIN(MainWindowTest)

#include "main_window_test.moc"
