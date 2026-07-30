/**
 * @file configuration_editor_dialog.hpp
 * @brief View/edit LogScope properties configuration (M14.12 Phase C).
 */

#pragma once

#include <QDialog>

#include "configuration_manager.hpp"

class QDialogButtonBox;
class QPushButton;
class QTableWidget;

namespace scope::desktop
{

/**
 * @brief Key/value editor for .properties configuration files.
 */
class ConfigurationEditorDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit ConfigurationEditorDialog(scope::configuration::ConfigurationManager& configurationManager,
                                     const QString& configFilePath, QWidget* parent = nullptr);

    /**
     * @brief Validates table contents without modifying the configuration manager.
     * @return Error message or empty if valid.
     */
    [[nodiscard]] QString validateCurrent() const;

    /**
     * @brief Applies table rows to the configuration manager (does not save to disk).
     */
    void applyToManager();

    [[nodiscard]] QString saveToFile(const QString& path);

    [[nodiscard]] bool configurationChanged() const noexcept { return m_configurationChanged; }

    [[nodiscard]] QString configFilePath() const { return m_configFilePath; }

  private:
    void buildUi();
    void populateTable();
    void addRow(const QString& key = {}, const QString& value = {});
    [[nodiscard]] scope::runtime::Configuration configurationFromTable() const;

    scope::configuration::ConfigurationManager& m_configurationManager;
    QString m_configFilePath;
    QTableWidget* m_table{nullptr};
    QPushButton* m_addButton{nullptr};
    QPushButton* m_validateButton{nullptr};
    QPushButton* m_saveButton{nullptr};
    QDialogButtonBox* m_buttons{nullptr};
    bool m_configurationChanged{false};
};

} // namespace scope::desktop
