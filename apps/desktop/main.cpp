/**
 * @file main.cpp
 * @brief LogScope desktop entry point.
 */

#include <QApplication>

#include <string>

#include "foundation/path.hpp"
#include "main_window.hpp"

int main(int argc, char* argv[])
{
    QApplication application(argc, argv);
    application.setApplicationName("LogScope");
    application.setOrganizationName("LogScope");

    scope::foundation::Path configPath;

    for (int index = 1; index < argc; ++index)
    {
        const std::string argument = argv[index];

        if (argument == "--config" && index + 1 < argc)
        {
            configPath = scope::foundation::Path(argv[++index]);
        }
    }

    scope::desktop::MainWindow window(configPath);
    window.show();

    return QApplication::exec();
}
