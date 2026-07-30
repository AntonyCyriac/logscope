/**
 * @file theme_manager.hpp
 * @brief Light/dark theme application for desktop UI.
 */

#pragma once

#include <QApplication>

namespace scope::desktop
{

enum class ThemeMode
{
    Light,
    Dark,
    System
};

/**
 * @brief Applies Qt palettes for light and dark themes.
 */
class ThemeManager
{
  public:
    static void apply(ThemeMode mode);
};

} // namespace scope::desktop
