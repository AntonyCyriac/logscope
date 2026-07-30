/**
 * @file main_window_test.cpp
 * @brief Desktop GUI integration tests (headless Qt Test).
 */

#include <QtTest>

#include "foundation/path.hpp"
#include "main_window.hpp"

namespace
{

QString sampleLogPath()
{
    return QStringLiteral("samples/sample.log");
}

QString noopConfigPath()
{
    return QStringLiteral("samples/ai-noop.properties");
}

} // namespace

class MainWindowTest : public QObject
{
    Q_OBJECT

  private slots:
    void openAndAnalyzeShowsEightLines();
    void askErrorsShowsFourMatches();
    void persistIndexStillShowsEightLines();
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

QTEST_MAIN(MainWindowTest)

#include "main_window_test.moc"
