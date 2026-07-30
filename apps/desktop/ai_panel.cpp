/**
 * @file ai_panel.cpp
 */

#include "ai_panel.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "ai_anomaly_hint_formatter.hpp"
#include "ai_summary_formatter.hpp"

namespace scope::desktop
{

AiPanel::AiPanel(scope::application::ApplicationService* service, QWidget* parent)
    : QWidget(parent), m_service(service)
{
    auto* layout = new QVBoxLayout(this);

    m_askEdit = new QLineEdit(this);
    m_askEdit->setPlaceholderText(QStringLiteral("Ask in natural language…"));

    auto* askRow = new QHBoxLayout();
    auto* askButton = new QPushButton(QStringLiteral("Ask"), this);
    m_summarizeButton = new QPushButton(QStringLiteral("Summarize"), this);
    m_hintsButton = new QPushButton(QStringLiteral("Hints"), this);

    askRow->addWidget(m_askEdit);
    askRow->addWidget(askButton);
    askRow->addWidget(m_summarizeButton);
    askRow->addWidget(m_hintsButton);

    m_outputEdit = new QTextEdit(this);
    m_outputEdit->setReadOnly(true);

    layout->addLayout(askRow);
    layout->addWidget(m_outputEdit);

    connect(askButton, &QPushButton::clicked, this, &AiPanel::runAsk);
    connect(m_summarizeButton, &QPushButton::clicked, this, &AiPanel::runSummarize);
    connect(m_hintsButton, &QPushButton::clicked, this, &AiPanel::runHints);
}

void AiPanel::runAsk()
{
    if (m_service == nullptr)
    {
        return;
    }

    scope::investigation::InvestigationCriteria criteria;
    const auto result = m_service->agentInvestigate(criteria, m_askEdit->text().toStdString(), false, false);

    if (!result)
    {
        m_outputEdit->setPlainText(QString::fromStdString(result.error().message()));

        return;
    }

    m_outputEdit->setPlainText(
        QStringLiteral("Matches: %1").arg(static_cast<qulonglong>(result->investigation.matchingLines.size())));
}

void AiPanel::runSummarize()
{
    if (m_service == nullptr)
    {
        return;
    }

    scope::investigation::InvestigationCriteria criteria;
    const auto result = m_service->agentInvestigate(criteria, std::string{}, true, false);

    if (!result)
    {
        m_outputEdit->setPlainText(QString::fromStdString(result.error().message()));

        return;
    }

    if (result->summary.has_value())
    {
        m_outputEdit->setPlainText(QString::fromStdString(scope::ai::formatAiSummary(*result->summary)));
    }
    else
    {
        m_outputEdit->setPlainText(QStringLiteral("No summary (ai.enabled=false or error)."));
    }
}

void AiPanel::runHints()
{
    if (m_service == nullptr)
    {
        return;
    }

    scope::investigation::InvestigationCriteria criteria;
    const auto result = m_service->agentInvestigate(criteria, std::string{}, false, true);

    if (!result)
    {
        m_outputEdit->setPlainText(QString::fromStdString(result.error().message()));

        return;
    }

    if (result->hints.has_value())
    {
        m_outputEdit->setPlainText(QString::fromStdString(scope::ai::formatAiAnomalyHints(*result->hints)));
    }
    else
    {
        m_outputEdit->setPlainText(QStringLiteral("No hints available."));
    }
}

} // namespace scope::desktop
