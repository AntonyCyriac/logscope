/**
 * @file theme_manager.cpp
 */

#include "theme_manager.hpp"

#include <QColor>
#include <QCoreApplication>
#include <QStyleFactory>

namespace scope::desktop
{

void ThemeManager::apply(const ThemeMode mode)
{
    QApplication* application = qobject_cast<QApplication*>(QCoreApplication::instance());

    if (application == nullptr)
    {
        return;
    }

    if (mode == ThemeMode::Dark)
    {
        application->setStyle(QStyleFactory::create("Fusion"));

        QPalette palette;
        palette.setColor(QPalette::Window, QColor(53, 53, 53));
        palette.setColor(QPalette::WindowText, Qt::white);
        palette.setColor(QPalette::Base, QColor(35, 35, 35));
        palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        palette.setColor(QPalette::Text, Qt::white);
        palette.setColor(QPalette::Button, QColor(53, 53, 53));
        palette.setColor(QPalette::ButtonText, Qt::white);
        palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        palette.setColor(QPalette::HighlightedText, Qt::black);
        application->setPalette(palette);
    }
    else
    {
        application->setStyle(QStyleFactory::create("Fusion"));
        application->setPalette(QApplication::style()->standardPalette());
    }
}

} // namespace scope::desktop
