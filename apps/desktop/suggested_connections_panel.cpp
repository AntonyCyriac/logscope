/**
 * @file suggested_connections_panel.cpp
 */

#include "suggested_connections_panel.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

namespace scope::desktop
{

SuggestedConnectionsPanel::SuggestedConnectionsPanel(QWidget* parent) : QWidget(parent)
{
    setupUi();
}

void SuggestedConnectionsPanel::setupUi()
{
    auto* layout = new QVBoxLayout(this);

    m_titleLabel = new QLabel(QStringLiteral("Suggested connections"), this);
    m_titleLabel->setObjectName(QStringLiteral("suggested-connections-title"));

    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("suggested-connections-panel"));

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_list, 1);

    setVisible(false);
}

void SuggestedConnectionsPanel::setSuggestions(
    const std::vector<scope::workspace::CorrelationSuggestion>& suggestions)
{
    m_suggestions = suggestions;
    renderSuggestions();
}

void SuggestedConnectionsPanel::clear()
{
    m_suggestions.clear();
    m_list->clear();
    setVisible(false);
}

bool SuggestedConnectionsPanel::isVisibleForTests() const
{
    return isVisible() && m_list->count() > 0;
}

int SuggestedConnectionsPanel::rowCount() const
{
    return m_list->count();
}

void SuggestedConnectionsPanel::renderSuggestions()
{
    m_list->clear();

    if (m_suggestions.empty())
    {
        setVisible(false);

        return;
    }

    for (const scope::workspace::CorrelationSuggestion& suggestion : m_suggestions)
    {
        auto* rowWidget = new QWidget(m_list);
        rowWidget->setObjectName(QStringLiteral("suggested-connection-row"));

        auto* rowLayout = new QHBoxLayout(rowWidget);
        auto* summary = new QLabel(QString::fromStdString(suggestion.summary), rowWidget);
        auto* acceptButton = new QPushButton(QStringLiteral("Accept"), rowWidget);
        acceptButton->setObjectName(QStringLiteral("suggested-connection-accept"));
        auto* dismissButton = new QPushButton(QStringLiteral("Dismiss"), rowWidget);
        dismissButton->setObjectName(QStringLiteral("suggested-connection-dismiss"));

        rowLayout->addWidget(summary, 1);
        rowLayout->addWidget(acceptButton);
        rowLayout->addWidget(dismissButton);

        const QString suggestionId = QString::fromStdString(suggestion.id);

        connect(acceptButton, &QPushButton::clicked, this, [this, suggestionId]() {
            emit acceptRequested(suggestionId);
        });
        connect(dismissButton, &QPushButton::clicked, this, [this, suggestionId]() {
            emit dismissRequested(suggestionId);
        });

        auto* item = new QListWidgetItem(m_list);
        item->setSizeHint(rowWidget->sizeHint());
        m_list->addItem(item);
        m_list->setItemWidget(item, rowWidget);
    }

    setVisible(true);
}

} // namespace scope::desktop
