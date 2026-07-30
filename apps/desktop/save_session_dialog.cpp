/**
 * @file save_session_dialog.cpp
 */

#include "save_session_dialog.hpp"

#include <limits>

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QVBoxLayout>

#include "line_count_filter.hpp"
#include "log_level_filter.hpp"

namespace scope::desktop
{

SaveSessionDialog::SaveSessionDialog(const scope::reporting::ReportOptions& defaultReportOptions, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Save Session"));
    buildUi(defaultReportOptions);
}

void SaveSessionDialog::buildUi(const scope::reporting::ReportOptions& defaults)
{
    auto* layout = new QVBoxLayout(this);

    auto* filtersGroup = new QGroupBox(QStringLiteral("Investigation filters"), this);
    auto* filtersForm = new QFormLayout(filtersGroup);

    m_minErrorsSpin = new QSpinBox(filtersGroup);
    m_minErrorsSpin->setObjectName(QStringLiteral("minErrorsSpin"));
    m_minErrorsSpin->setRange(0, static_cast<int>(std::numeric_limits<std::uint32_t>::max()));
    filtersForm->addRow(QStringLiteral("Min errors"), m_minErrorsSpin);

    m_minWarningsSpin = new QSpinBox(filtersGroup);
    m_minWarningsSpin->setObjectName(QStringLiteral("minWarningsSpin"));
    m_minWarningsSpin->setRange(0, static_cast<int>(std::numeric_limits<std::uint32_t>::max()));
    filtersForm->addRow(QStringLiteral("Min warnings"), m_minWarningsSpin);

    m_minLinesSpin = new QSpinBox(filtersGroup);
    m_minLinesSpin->setObjectName(QStringLiteral("minLinesSpin"));
    m_minLinesSpin->setRange(0, static_cast<int>(std::numeric_limits<std::uint32_t>::max()));
    filtersForm->addRow(QStringLiteral("Min lines"), m_minLinesSpin);

    m_maxLinesSpin = new QSpinBox(filtersGroup);
    m_maxLinesSpin->setObjectName(QStringLiteral("maxLinesSpin"));
    m_maxLinesSpin->setRange(0, static_cast<int>(std::numeric_limits<std::uint32_t>::max()));
    m_maxLinesSpin->setSpecialValueText(QStringLiteral("(none)"));
    filtersForm->addRow(QStringLiteral("Max lines"), m_maxLinesSpin);

    m_searchEdit = new QLineEdit(filtersGroup);
    m_searchEdit->setPlaceholderText(QStringLiteral("Source path search"));
    filtersForm->addRow(QStringLiteral("Search"), m_searchEdit);

    m_includeInvestigationCheck = new QCheckBox(QStringLiteral("Include current investigation bar filters"), filtersGroup);
    m_includeInvestigationCheck->setChecked(true);
    filtersForm->addRow(m_includeInvestigationCheck);

    layout->addWidget(filtersGroup);

    auto* reportGroup = new QGroupBox(QStringLiteral("Report"), this);
    auto* reportLayout = new QVBoxLayout(reportGroup);

    m_formatCombo = new QComboBox(reportGroup);
    m_formatCombo->addItems({QStringLiteral("text"), QStringLiteral("html"), QStringLiteral("pdf"),
                             QStringLiteral("json"), QStringLiteral("markdown"), QStringLiteral("csv")});
    reportLayout->addWidget(m_formatCombo);

    auto* sectionsLayout = new QVBoxLayout();
    const scope::reporting::ReportSections& sections = defaults.sections;

    m_executiveCheck = new QCheckBox(QStringLiteral("Executive summary"), reportGroup);
    m_executiveCheck->setChecked(sections.includes(scope::reporting::ReportSection::ExecutiveSummary));
    m_summaryCheck = new QCheckBox(QStringLiteral("Summary"), reportGroup);
    m_summaryCheck->setChecked(sections.includes(scope::reporting::ReportSection::Summary));
    m_levelsCheck = new QCheckBox(QStringLiteral("Level breakdown"), reportGroup);
    m_levelsCheck->setChecked(sections.includes(scope::reporting::ReportSection::LevelBreakdown));
    m_errorsCheck = new QCheckBox(QStringLiteral("Error summary"), reportGroup);
    m_errorsCheck->setChecked(sections.includes(scope::reporting::ReportSection::ErrorSummary));
    m_analyticsCheck = new QCheckBox(QStringLiteral("Analytics summary"), reportGroup);
    m_analyticsCheck->setChecked(sections.includes(scope::reporting::ReportSection::AnalyticsSummary));
    m_timelineCheck = new QCheckBox(QStringLiteral("Timeline"), reportGroup);
    m_timelineCheck->setChecked(sections.includes(scope::reporting::ReportSection::Timeline));
    m_clustersCheck = new QCheckBox(QStringLiteral("Clusters"), reportGroup);
    m_clustersCheck->setChecked(sections.includes(scope::reporting::ReportSection::Clusters));
    m_chartsCheck = new QCheckBox(QStringLiteral("Charts"), reportGroup);
    m_chartsCheck->setChecked(sections.includes(scope::reporting::ReportSection::Charts));
    m_metadataCheck = new QCheckBox(QStringLiteral("Source metadata"), reportGroup);
    m_metadataCheck->setChecked(sections.includes(scope::reporting::ReportSection::SourceMetadata));
    m_formatsFooterCheck = new QCheckBox(QStringLiteral("Formats footer"), reportGroup);
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
    reportLayout->addLayout(sectionsLayout);

    layout->addWidget(reportGroup);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

scope::application::SessionSaveRequest SaveSessionDialog::sessionSaveRequest(
    const scope::foundation::Path& sessionFile, const scope::foundation::Path& configFile,
    const scope::investigation::InvestigationCriteria& contentCriteria) const
{
    scope::application::SessionSaveRequest request;
    request.sessionFile = sessionFile;
    request.configFile = configFile;

    const auto* minErrorsSpin = findChild<QSpinBox*>(QStringLiteral("minErrorsSpin"));
    const auto* minWarningsSpin = findChild<QSpinBox*>(QStringLiteral("minWarningsSpin"));
    const auto* minLinesSpin = findChild<QSpinBox*>(QStringLiteral("minLinesSpin"));
    const auto* maxLinesSpin = findChild<QSpinBox*>(QStringLiteral("maxLinesSpin"));

    const int minErrorsValue = minErrorsSpin != nullptr ? minErrorsSpin->value() : 0;
    const int minWarningsValue = minWarningsSpin != nullptr ? minWarningsSpin->value() : 0;
    const int minLinesValue = minLinesSpin != nullptr ? minLinesSpin->value() : 0;
    const int maxLinesValue = maxLinesSpin != nullptr ? maxLinesSpin->value() : 0;

    scope::investigation::LineCountFilter lineFilter =
        scope::investigation::LineCountFilter::any().withMinimum(static_cast<std::uint64_t>(minLinesValue));

    if (maxLinesValue > 0)
    {
        lineFilter = lineFilter.withMaximum(static_cast<std::uint64_t>(maxLinesValue));
    }

    request.lineFilter = lineFilter;
    request.levelFilter = scope::investigation::LogLevelFilter::any()
                              .withMinimumErrors(static_cast<std::uint64_t>(minErrorsValue))
                              .withMinimumWarnings(static_cast<std::uint64_t>(minWarningsValue));
    request.searchQuery = m_searchEdit != nullptr ? m_searchEdit->text().toStdString() : std::string{};

    if (m_includeInvestigationCheck->isChecked())
    {
        request.contentCriteria = contentCriteria;
    }

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
    request.reportOptions = options;

    return request;
}

} // namespace scope::desktop
