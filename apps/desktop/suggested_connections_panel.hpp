/**
 * @file suggested_connections_panel.hpp
 * @brief Story 6 Suggested connections panel (P2.1).
 */

#pragma once

#include <QWidget>

#include <vector>

#include "correlation_suggestion.hpp"

class QLabel;
class QListWidget;

namespace scope::desktop
{

/**
 * @brief Ephemeral correlation suggestions for the selected timeline event.
 */
class SuggestedConnectionsPanel : public QWidget
{
    Q_OBJECT

  public:
    explicit SuggestedConnectionsPanel(QWidget* parent = nullptr);

    void setSuggestions(const std::vector<scope::workspace::CorrelationSuggestion>& suggestions);

    void clear();

    [[nodiscard]] bool isVisibleForTests() const;
    [[nodiscard]] int rowCount() const;

  signals:
    void acceptRequested(const QString& suggestionId);
    void dismissRequested(const QString& suggestionId);

  private:
    void setupUi();
    void renderSuggestions();

    std::vector<scope::workspace::CorrelationSuggestion> m_suggestions;
    QLabel* m_titleLabel{nullptr};
    QListWidget* m_list{nullptr};
};

} // namespace scope::desktop
