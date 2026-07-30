/**
 * @file open_log_dialog.hpp
 * @brief Open log file with format and profile overrides (M14.12 Phase C).
 */

#pragma once

#include <QDialog>

#include <string>

#include "log_format.hpp"

class QComboBox;
class QDialogButtonBox;
class QLineEdit;
class QPushButton;

namespace scope::desktop
{

/**
 * @brief File picker with log-format and profile options (CLI --log-format / --profile).
 */
class OpenLogDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit OpenLogDialog(QWidget* parent = nullptr);

    [[nodiscard]] QString logPath() const;
    [[nodiscard]] scope::analysis::LogFormat logFormat() const;
    [[nodiscard]] std::string profile() const;

  private:
    void buildUi();

    QLineEdit* m_pathEdit{nullptr};
    QComboBox* m_formatCombo{nullptr};
    QComboBox* m_profileCombo{nullptr};
};

} // namespace scope::desktop
