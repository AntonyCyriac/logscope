/**
 * @file analytics_panel.cpp
 */

#include "analytics_panel.hpp"

#include <QString>

namespace scope::desktop
{

AnalyticsPanel::AnalyticsPanel(QWidget* parent) : QTabWidget(parent)
{
    m_timelineEdit = new QTextEdit(this);
    m_timelineEdit->setReadOnly(true);
    m_frequencyEdit = new QTextEdit(this);
    m_frequencyEdit->setReadOnly(true);
    m_clustersEdit = new QTextEdit(this);
    m_clustersEdit->setReadOnly(true);
    m_correlationsEdit = new QTextEdit(this);
    m_correlationsEdit->setReadOnly(true);

    addTab(m_timelineEdit, QStringLiteral("Timeline"));
    addTab(m_frequencyEdit, QStringLiteral("Frequencies"));
    addTab(m_clustersEdit, QStringLiteral("Clusters"));
    addTab(m_correlationsEdit, QStringLiteral("Correlations"));
}

void AnalyticsPanel::showAnalytics(const scope::analytics::AnalyticsResult& result)
{
    QString timelineText;

    for (const auto& bucket : result.timeline().buckets())
    {
        timelineText += QStringLiteral("%1: %2 lines\n")
                            .arg(QString::fromStdString(bucket.label))
                            .arg(static_cast<qulonglong>(bucket.totalLines));
    }

    m_timelineEdit->setPlainText(timelineText);

    QString frequencyText;

    for (const auto& entry : result.frequency().topMessages())
    {
        frequencyText += QStringLiteral("%1: %2\n")
                              .arg(QString::fromStdString(entry.key))
                              .arg(static_cast<qulonglong>(entry.count));
    }

    m_frequencyEdit->setPlainText(frequencyText);

    QString clusterText;

    for (const auto& cluster : result.clusters().clusters())
    {
        clusterText += QStringLiteral("%1 (%2)\n")
                           .arg(QString::fromStdString(cluster.signature))
                           .arg(static_cast<qulonglong>(cluster.count));
    }

    m_clustersEdit->setPlainText(clusterText);

    QString correlationText;

    for (const auto& item : result.correlations().repeatedErrors())
    {
        correlationText += QStringLiteral("%1 (%2)\n")
                                .arg(QString::fromStdString(item.key))
                                .arg(static_cast<qulonglong>(item.count));
    }

    m_correlationsEdit->setPlainText(correlationText);
}

} // namespace scope::desktop
