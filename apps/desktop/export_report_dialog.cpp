/**
 * @file export_report_dialog.cpp
 */

#include "export_report_dialog.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QVBoxLayout>

namespace scope::desktop
{

ExportReportDialog::ExportReportDialog(const scope::reporting::ReportOptions& defaults, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Export Report"));
    buildUi(defaults);
}

void ExportReportDialog::buildUi(const scope::reporting::ReportOptions& defaults)
{
    auto* layout = new QVBoxLayout(this);

    auto* formatRow = new QFormLayout();
    m_formatCombo = new QComboBox(this);
    m_formatCombo->addItems({QStringLiteral("text"), QStringLiteral("html"), QStringLiteral("pdf"),
                             QStringLiteral("json"), QStringLiteral("markdown"), QStringLiteral("csv")});

    switch (defaults.format)
    {
    case scope::reporting::ReportFormat::Html:
        m_formatCombo->setCurrentText(QStringLiteral("html"));
        break;
    case scope::reporting::ReportFormat::Pdf:
        m_formatCombo->setCurrentText(QStringLiteral("pdf"));
        break;
    case scope::reporting::ReportFormat::Json:
        m_formatCombo->setCurrentText(QStringLiteral("json"));
        break;
    case scope::reporting::ReportFormat::Markdown:
        m_formatCombo->setCurrentText(QStringLiteral("markdown"));
        break;
    case scope::reporting::ReportFormat::Csv:
        m_formatCombo->setCurrentText(QStringLiteral("csv"));
        break;
    default:
        m_formatCombo->setCurrentText(QStringLiteral("text"));
        break;
    }

    formatRow->addRow(QStringLiteral("Format"), m_formatCombo);
    layout->addLayout(formatRow);

    auto* sectionsGroup = new QGroupBox(QStringLiteral("Sections"), this);
    auto* sectionsLayout = new QVBoxLayout(sectionsGroup);

    const scope::reporting::ReportSections& sections = defaults.sections;

    m_executiveCheck = new QCheckBox(QStringLiteral("Executive summary"), sectionsGroup);
    m_executiveCheck->setChecked(sections.includes(scope::reporting::ReportSection::ExecutiveSummary));
    m_summaryCheck = new QCheckBox(QStringLiteral("Summary"), sectionsGroup);
    m_summaryCheck->setChecked(sections.includes(scope::reporting::ReportSection::Summary));
    m_levelsCheck = new QCheckBox(QStringLiteral("Level breakdown"), sectionsGroup);
    m_levelsCheck->setChecked(sections.includes(scope::reporting::ReportSection::LevelBreakdown));
    m_errorsCheck = new QCheckBox(QStringLiteral("Error summary"), sectionsGroup);
    m_errorsCheck->setChecked(sections.includes(scope::reporting::ReportSection::ErrorSummary));
    m_analyticsCheck = new QCheckBox(QStringLiteral("Analytics summary"), sectionsGroup);
    m_analyticsCheck->setChecked(sections.includes(scope::reporting::ReportSection::AnalyticsSummary));
    m_timelineCheck = new QCheckBox(QStringLiteral("Timeline"), sectionsGroup);
    m_timelineCheck->setChecked(sections.includes(scope::reporting::ReportSection::Timeline));
    m_clustersCheck = new QCheckBox(QStringLiteral("Clusters"), sectionsGroup);
    m_clustersCheck->setChecked(sections.includes(scope::reporting::ReportSection::Clusters));
    m_chartsCheck = new QCheckBox(QStringLiteral("Charts"), sectionsGroup);
    m_chartsCheck->setChecked(sections.includes(scope::reporting::ReportSection::Charts));
    m_metadataCheck = new QCheckBox(QStringLiteral("Source metadata"), sectionsGroup);
    m_metadataCheck->setChecked(sections.includes(scope::reporting::ReportSection::SourceMetadata));
    m_formatsFooterCheck = new QCheckBox(QStringLiteral("Formats footer"), sectionsGroup);
    m_formatsFooterCheck->setChecked(sections.includes(scope::reporting::ReportSection::FormatsFooter));

    sectionsLayout->addWidget(m_executiveCheck);
    sectionsLayout->addWidget(m_summaryCheck);
    sectionsLayout->addWidget(m_levelsCheck);
    sectionsLayout->addWidget(m_errorsCheck);
    sectionsLayout->addWidget(m_analyticsCheck);
    sectionsLayout->addWidget(m_timelineCheck);
    sectionsLayout->addWidget(m_clustersCheck);
    sectionsLayout->addWidget(m_chartsCheck);
    sectionsLayout->addWidget(m_metadataCheck);
    sectionsLayout->addWidget(m_formatsFooterCheck);

    layout->addWidget(sectionsGroup);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

scope::reporting::ReportOptions ExportReportDialog::reportOptions() const
{
    scope::reporting::ReportOptions options;
    const QString format = m_formatCombo->currentText();

    if (format == QStringLiteral("html"))
    {
        options.format = scope::reporting::ReportFormat::Html;
    }
    else if (format == QStringLiteral("pdf"))
    {
        options.format = scope::reporting::ReportFormat::Pdf;
    }
    else if (format == QStringLiteral("json"))
    {
        options.format = scope::reporting::ReportFormat::Json;
    }
    else if (format == QStringLiteral("markdown"))
    {
        options.format = scope::reporting::ReportFormat::Markdown;
    }
    else if (format == QStringLiteral("csv"))
    {
        options.format = scope::reporting::ReportFormat::Csv;
    }
    else
    {
        options.format = scope::reporting::ReportFormat::Text;
    }

    scope::reporting::ReportSections sections;

    if (m_executiveCheck->isChecked())
    {
        sections.enable(scope::reporting::ReportSection::ExecutiveSummary);
    }

    if (m_summaryCheck->isChecked())
    {
        sections.enable(scope::reporting::ReportSection::Summary);
    }

    if (m_levelsCheck->isChecked())
    {
        sections.enable(scope::reporting::ReportSection::LevelBreakdown);
    }

    if (m_errorsCheck->isChecked())
    {
        sections.enable(scope::reporting::ReportSection::ErrorSummary);
    }

    if (m_analyticsCheck->isChecked())
    {
        sections.enable(scope::reporting::ReportSection::AnalyticsSummary);
    }

    if (m_timelineCheck->isChecked())
    {
        sections.enable(scope::reporting::ReportSection::Timeline);
    }

    if (m_clustersCheck->isChecked())
    {
        sections.enable(scope::reporting::ReportSection::Clusters);
    }

    if (m_chartsCheck->isChecked())
    {
        sections.enable(scope::reporting::ReportSection::Charts);
    }

    if (m_metadataCheck->isChecked())
    {
        sections.enable(scope::reporting::ReportSection::SourceMetadata);
    }

    if (m_formatsFooterCheck->isChecked())
    {
        sections.enable(scope::reporting::ReportSection::FormatsFooter);
    }

    options.sections = sections;

    return options;
}

} // namespace scope::desktop
