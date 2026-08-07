/**
 * @file capture_readme_screenshot.cpp
 * @brief Headless capture of README desktop screenshot (offscreen Qt).
 */

#include <QApplication>
#include <QElapsedTimer>
#include <QPixmap>
#include <QThread>
#include <QWindow>

#include "foundation/path.hpp"
#include "main_window.hpp"

namespace
{

bool waitForWindowExposed(QWidget* widget, const int timeoutMs = 10000)
{
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < timeoutMs) {
        QApplication::processEvents();
        if (widget->isVisible() && widget->windowHandle() != nullptr && widget->windowHandle()->isExposed()) {
            return true;
        }
        QThread::msleep(10);
    }
    return widget->windowHandle() != nullptr && widget->windowHandle()->isExposed();
}

} // namespace

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    scope::desktop::MainWindow window(scope::foundation::Path("samples/ai-noop.properties"));
    window.resize(1280, 800);
    window.show();

    if (!waitForWindowExposed(&window)) {
        return 1;
    }

    if (!window.openLogFile(QStringLiteral("samples/sample.log"))) {
        return 2;
    }

    QApplication::processEvents();

    const QPixmap screenshot = window.grab();
    if (screenshot.isNull()) {
        return 3;
    }

    const QString outputPath = QStringLiteral("docs/assets/logscope-desktop.png");
    if (!screenshot.save(outputPath)) {
        return 4;
    }

    return 0;
}
