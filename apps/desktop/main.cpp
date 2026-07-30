/**
 * @file main.cpp
 * @brief LogScope desktop entry point.
 */

#include <QApplication>

#include "main_window.hpp"

int main(int argc, char* argv[])
{
    QApplication application(argc, argv);
    application.setApplicationName("LogScope");
    application.setOrganizationName("LogScope");

    scope::desktop::MainWindow window;
    window.show();

    return QApplication::exec();
}
