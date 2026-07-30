/**
 * @file analytics_panel.hpp
 */

#pragma once

#include <QTabWidget>
#include <QTextEdit>

#include "analytics_result.hpp"

namespace scope::desktop
{

/**
 * @brief Displays analytics results in tabbed text panels.
 */
class AnalyticsPanel : public QTabWidget
{
    Q_OBJECT

  public:
    explicit AnalyticsPanel(QWidget* parent = nullptr);

    void showAnalytics(const scope::analytics::AnalyticsResult& result);

  private:
    QTextEdit* m_timelineEdit{nullptr};
    QTextEdit* m_frequencyEdit{nullptr};
    QTextEdit* m_clustersEdit{nullptr};
    QTextEdit* m_correlationsEdit{nullptr};
};

} // namespace scope::desktop
