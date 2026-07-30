/**
 * @file run_stats_dialog.hpp
 * @brief Displays CLI-equivalent --stats output in the desktop UI.
 */

#pragma once

#include <QDialog>

#include "analysis_stats.hpp"
#include "plugin_load_stats.hpp"

namespace scope::desktop
{

class RunStatsDialog : public QDialog
{
    Q_OBJECT

  public:
    RunStatsDialog(const scope::analysis::AnalysisStats& analysisStats,
                   const scope::plugin::PluginLoadStats& pluginStats,
                   QWidget* parent = nullptr);
};

} // namespace scope::desktop
