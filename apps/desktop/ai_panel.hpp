/**
 * @file ai_panel.hpp
 */

#pragma once

#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>

#include "application_service.hpp"
#include "investigation_result.hpp"

namespace scope::desktop
{

/**
 * @brief AI assistant panel (ask, summarize, hints).
 */
class AiPanel : public QWidget
{
    Q_OBJECT

  public:
    explicit AiPanel(scope::application::ApplicationService* service, QWidget* parent = nullptr);

    [[nodiscard]] QString outputText() const;

    /// Sets Ask text and runs Ask (used by desktop integration tests).
    void submitAsk(const QString& query);

  signals:
    void investigationReady(const scope::investigation::InvestigationResult& result);

  private:
    void runAsk();
    void runSummarize();
    void runHints();

    scope::application::ApplicationService* m_service{nullptr};
    QLineEdit* m_askEdit{nullptr};
    QTextEdit* m_outputEdit{nullptr};
    QPushButton* m_summarizeButton{nullptr};
    QPushButton* m_hintsButton{nullptr};
};

} // namespace scope::desktop
