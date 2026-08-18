/**
 * @file investigation_parity_test.cpp
 * @brief P2 Story Gate — desktop Timeline + Crash parity (headless).
 */

#include <QtTest>

#include "foundation/path.hpp"
#include "main_window.hpp"

namespace
{

QString investigationsRoot()
{
    return QStringLiteral("build/desktop_parity_investigations");
}

QString sampleLogPath()
{
    return QStringLiteral("samples/sample.log");
}

QString pstackPath()
{
    return QStringLiteral("samples/pstack.txt");
}

QString noopConfigPath()
{
    return QStringLiteral("samples/ai-noop.properties");
}

scope::desktop::MainWindow makeWindow()
{
    return scope::desktop::MainWindow(scope::foundation::Path(noopConfigPath().toStdString()),
                                      scope::foundation::Path(investigationsRoot().toStdString()));
}

} // namespace

class InvestigationParityTest : public QObject
{
    Q_OBJECT

  private slots:
    void investigationTabsDisabledWithoutInvestigation();
    void crashTabDisabledUntilPstackArtifact();
    void openInvestigationRoundTrip();
    void storyGateTimelineCrashAndFaultThreadJump();
};

void InvestigationParityTest::investigationTabsDisabledWithoutInvestigation()
{
    scope::desktop::MainWindow window = makeWindow();
    window.show();
    QTest::qWait(50);

    QVERIFY(!window.investigationModeActive());
    QVERIFY(!window.isBottomTabEnabled(QStringLiteral("Timeline")));
    QVERIFY(!window.isBottomTabEnabled(QStringLiteral("Crash")));
    QCOMPARE(window.currentBottomTabName(), QStringLiteral("Results"));
}

void InvestigationParityTest::crashTabDisabledUntilPstackArtifact()
{
    scope::desktop::MainWindow window = makeWindow();
    window.show();
    QTest::qWait(50);

    QVERIFY(window.createInvestigation(QStringLiteral("parity-crash-tab")));
    QVERIFY(window.isBottomTabEnabled(QStringLiteral("Timeline")));
    QVERIFY(!window.isBottomTabEnabled(QStringLiteral("Crash")));

    QVERIFY(window.addInvestigationLogArtifact(sampleLogPath()));
    QVERIFY(!window.isBottomTabEnabled(QStringLiteral("Crash")));

    QVERIFY(window.addInvestigationPstackArtifact(pstackPath()));
    QVERIFY(window.isBottomTabEnabled(QStringLiteral("Crash")));
    QCOMPARE(window.investigationArtifactCount(), 2);
}

void InvestigationParityTest::openInvestigationRoundTrip()
{
    scope::desktop::MainWindow window = makeWindow();
    window.show();
    QTest::qWait(50);

    QVERIFY(window.createInvestigation(QStringLiteral("parity-open")));
    QVERIFY(window.addInvestigationLogArtifact(sampleLogPath()));
    QVERIFY(window.addInvestigationPstackArtifact(pstackPath()));

    const QString investigationPath = window.investigationDirectoryPath();
    QVERIFY(!investigationPath.isEmpty());

    QVERIFY(window.closeActiveInvestigation());
    QVERIFY(!window.investigationModeActive());

    QVERIFY(window.openInvestigationAtPath(investigationPath));
    QCOMPARE(window.investigationArtifactCount(), 2);

    QVERIFY(window.switchBottomTab(QStringLiteral("Timeline")));
    QTest::qWait(500);
    QVERIFY(window.timelineRowCount() > 0);
    QVERIFY(window.timelineHasEventType(QStringLiteral("crash.summary")));
}

void InvestigationParityTest::storyGateTimelineCrashAndFaultThreadJump()
{
    scope::desktop::MainWindow window = makeWindow();
    window.show();
    QTest::qWait(100);

    QVERIFY(window.createInvestigation(QStringLiteral("parity-test")));
    QTest::qWait(50);

    QVERIFY(window.addInvestigationLogArtifact(sampleLogPath()));
    QTest::qWait(50);

    QVERIFY(window.addInvestigationPstackArtifact(pstackPath()));
    QTest::qWait(200);

    QVERIFY(window.switchBottomTab(QStringLiteral("Timeline")));
    QTest::qWait(500);

    QVERIFY(window.timelineHasEventType(QStringLiteral("crash.summary")));
    QVERIFY(window.selectTimelineCrashSummary());
    QTest::qWait(200);

    QCOMPARE(window.currentBottomTabName(), QStringLiteral("Crash"));
    QVERIFY(window.crashSignalText().contains(QStringLiteral("SIGSEGV")));

    QVERIFY(window.clickCrashFaultThread());
    QVERIFY(window.statusMessage().contains(QStringLiteral("Jumped to pstack thread")));
}

QTEST_MAIN(InvestigationParityTest)

#include "investigation_parity_test.moc"
