/**
 * @file configuration_editor_dialog.cpp
 */

#include "configuration_editor_dialog.hpp"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include "analysis_config.hpp"

namespace scope::desktop
{

ConfigurationEditorDialog::ConfigurationEditorDialog(
    scope::configuration::ConfigurationManager& configurationManager, const QString& configFilePath,
    QWidget* parent)
    : QDialog(parent), m_configurationManager(configurationManager), m_configFilePath(configFilePath)
{
    setWindowTitle(QStringLiteral("Configuration"));
    resize(640, 480);
    buildUi();
    populateTable();
}

void ConfigurationEditorDialog::buildUi()
{
    auto* layout = new QVBoxLayout(this);

    m_table = new QTableWidget(0, 2, this);
    m_table->setHorizontalHeaderLabels({QStringLiteral("Key"), QStringLiteral("Value")});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(m_table);

    auto* buttonRow = new QHBoxLayout();
    m_addButton = new QPushButton(QStringLiteral("Add row"), this);
    m_validateButton = new QPushButton(QStringLiteral("Validate"), this);
    m_saveButton = new QPushButton(QStringLiteral("Save…"), this);
    buttonRow->addWidget(m_addButton);
    buttonRow->addWidget(m_validateButton);
    buttonRow->addWidget(m_saveButton);
    buttonRow->addStretch();
    layout->addLayout(buttonRow);

    m_buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    layout->addWidget(m_buttons);

    connect(m_addButton, &QPushButton::clicked, this, [this]() { addRow(); });
    connect(m_validateButton, &QPushButton::clicked, this, [this]() {
        const QString error = validateCurrent();

        if (error.isEmpty())
        {
            QMessageBox::information(this, QStringLiteral("Configuration"), QStringLiteral("Configuration is valid."));
        }
        else
        {
            QMessageBox::warning(this, QStringLiteral("Configuration"), error);
        }
    });
    connect(m_saveButton, &QPushButton::clicked, this, [this]() {
        const QString validationError = validateCurrent();

        if (!validationError.isEmpty())
        {
            QMessageBox::warning(this, QStringLiteral("Configuration"), validationError);

            return;
        }

        QString path = m_configFilePath;

        if (path.isEmpty())
        {
            path = QFileDialog::getSaveFileName(this, QStringLiteral("Save configuration"), QString{},
                                                QStringLiteral("Properties (*.properties)"));
        }

        if (path.isEmpty())
        {
            return;
        }

        const QString saveError = saveToFile(path);

        if (!saveError.isEmpty())
        {
            QMessageBox::warning(this, QStringLiteral("Save failed"), saveError);

            return;
        }

        m_configFilePath = path;
        m_configurationChanged = true;
        QMessageBox::information(this, QStringLiteral("Configuration"),
                                 QStringLiteral("Saved to %1").arg(path));
    });
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void ConfigurationEditorDialog::populateTable()
{
    m_table->setRowCount(0);

    const std::vector<std::string> keys = m_configurationManager.configuration().keys();

    for (const std::string& key : keys)
    {
        const auto value = m_configurationManager.configuration().get(key);

        if (value)
        {
            addRow(QString::fromStdString(key), QString::fromStdString(*value));
        }
    }
}

void ConfigurationEditorDialog::addRow(const QString& key, const QString& value)
{
    const int row = m_table->rowCount();
    m_table->insertRow(row);
    m_table->setItem(row, 0, new QTableWidgetItem(key));
    m_table->setItem(row, 1, new QTableWidgetItem(value));
}

scope::runtime::Configuration ConfigurationEditorDialog::configurationFromTable() const
{
    scope::runtime::Configuration configuration;

    for (int row = 0; row < m_table->rowCount(); ++row)
    {
        const QTableWidgetItem* keyItem = m_table->item(row, 0);
        const QTableWidgetItem* valueItem = m_table->item(row, 1);

        if (keyItem == nullptr || keyItem->text().trimmed().isEmpty())
        {
            continue;
        }

        const std::string key = keyItem->text().trimmed().toStdString();
        const std::string value = valueItem != nullptr ? valueItem->text().toStdString() : std::string{};

        configuration.set(key, value);
    }

    return configuration;
}

QString ConfigurationEditorDialog::validateCurrent() const
{
    const scope::runtime::Configuration configuration = configurationFromTable();
    scope::configuration::ConfigurationManager scratch;

    scratch.configuration() = configuration;

    const auto basicResult = scratch.validate({});

    if (!basicResult)
    {
        return QString::fromStdString(basicResult.error().message());
    }

    const auto analysisResult = scope::analysis::validateAnalysisConfiguration(configuration);

    if (!analysisResult)
    {
        return QString::fromStdString(analysisResult.error().message());
    }

    return {};
}

void ConfigurationEditorDialog::applyToManager()
{
    m_configurationManager.configuration() = configurationFromTable();
}

QString ConfigurationEditorDialog::saveToFile(const QString& path)
{
    applyToManager();

    const auto saveResult = m_configurationManager.saveToFile(scope::foundation::Path(path.toStdString()));

    if (!saveResult)
    {
        return QString::fromStdString(saveResult.error().message());
    }

    return {};
}

} // namespace scope::desktop
