/**
 * @file save_session_dialog.hpp
 * @brief Session save options mirroring CLI session save (M14.12 Phase C).
 */

#pragma once

#include <QDialog>

#include "application_service.hpp"
#include "report_options.hpp"

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QLineEdit;
class QSpinBox;

namespace scope::desktop
{

/**
 * @brief Options for saving an investigation session (filters + report sections).
 */
class SaveSessionDialog : public QDialog
{
  public:
    SaveSessionDialog(const scope::reporting::ReportOptions& defaultReportOptions, QWidget* parent = nullptr);

    [[nodiscard]] scope::application::SessionSaveRequest sessionSaveRequest(
        const scope::foundation::Path& sessionFile, const scope::foundation::Path& configFile,
        const scope::investigation::InvestigationCriteria& contentCriteria) const;

  private:
    void buildUi(const scope::reporting::ReportOptions& defaults);

    QSpinBox* m_minErrorsSpin{nullptr};
    QSpinBox* m_minWarningsSpin{nullptr};
    QSpinBox* m_minLinesSpin{nullptr};
    QSpinBox* m_maxLinesSpin{nullptr};
    QLineEdit* m_searchEdit{nullptr};
    QCheckBox* m_includeInvestigationCheck{nullptr};
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
