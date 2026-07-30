/**
 * @file export_report_dialog.hpp
 * @brief Report export options (format + sections) for desktop.
 */

#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>

#include "report_options.hpp"

namespace scope::desktop
{

/**
 * @brief Modal dialog to pick report format and sections before export.
 */
class ExportReportDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit ExportReportDialog(const scope::reporting::ReportOptions& defaults, QWidget* parent = nullptr);

    [[nodiscard]] scope::reporting::ReportOptions reportOptions() const;

  private:
    void buildUi(const scope::reporting::ReportOptions& defaults);

    QComboBox* m_formatCombo{nullptr};
    QCheckBox* m_executiveCheck{nullptr};
    QCheckBox* m_summaryCheck{nullptr};
    QCheckBox* m_levelsCheck{nullptr};
    QCheckBox* m_errorsCheck{nullptr};
    QCheckBox* m_analyticsCheck{nullptr};
    QCheckBox* m_timelineCheck{nullptr};
    QCheckBox* m_clustersCheck{nullptr};
    QCheckBox* m_chartsCheck{nullptr};
    QCheckBox* m_metadataCheck{nullptr};
    QCheckBox* m_formatsFooterCheck{nullptr};
};

} // namespace scope::desktop
